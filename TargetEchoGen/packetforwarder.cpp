#include "packetforwarder.h"
#include "log.h"
#include <pcap.h>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
static constexpr size_t STRUCT_SIZE = sizeof(UdpPayload);
// ---------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------
PacketForwarder::PacketForwarder(const QString &interfaceName,
                                 quint16 listenUdpPort,
                                 const QString &forwardIp,
                                 quint16 forwardUdpPort,
                                 const QString &dumpFilePath,
                                 QObject *parent)
    : QThread(parent),
    m_interfaceName(interfaceName),
    m_listenUdpPort(listenUdpPort),
    m_forwardIp(forwardIp),
    m_forwardUdpPort(forwardUdpPort),
    m_dumpFilePath(dumpFilePath),
    m_stopRequested(false),
    m_pcapHandle(nullptr),
    m_pcapDumper(nullptr),
    m_udpSock(-1)
#ifdef _WIN32
    , m_wsaInitialized(false)
#endif
{
    Xtp = 0.0;
    Ytp = 0.0;
    Ztp = 0.0;
}

PacketForwarder::~PacketForwarder()
{
    stop();
    wait();     // wait for thread to exit cleanly
}

// ---------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------
void PacketForwarder::stop()
{
    m_stopRequested.store(true, std::memory_order_relaxed);
}

// ---------------------------------------------------------------------
// Thread entry
// ---------------------------------------------------------------------
void PacketForwarder::run()
{
    if (!initialize()) {
        LOG_ERROR("PacketForwarder: initialization failed, exiting thread.");
        cleanup();  // in case of partial init
        return;
    }

    LOG_INFO("PacketForwarder: initialization OK, entering capture loop.");
    captureLoop();
    cleanup();
    LOG_INFO("PacketForwarder: thread exited.");
}

bool PacketForwarder::applyFilter(const QString &filterString)
{
    if (!m_pcapHandle)
        return false;

    QByteArray filterBA = filterString.toLocal8Bit();

    struct bpf_program fp;
    bpf_u_int32 netmask = PCAP_NETMASK_UNKNOWN;

    if (pcap_compile(m_pcapHandle, &fp, filterBA.constData(), 1, netmask) == -1) {
        LOG_ERROR("pcap_compile failed: %s", pcap_geterr(m_pcapHandle));
        return false;
    }

    if (pcap_setfilter(m_pcapHandle, &fp) == -1) {
        LOG_ERROR("pcap_setfilter failed: %s", pcap_geterr(m_pcapHandle));
        pcap_freecode(&fp);
        return false;
    }

    pcap_freecode(&fp);
    return true;
}
bool PacketForwarder::openLogFile(const std::string &path)
{
    g_logFile.open(path, std::ios::out | std::ios::app | std::ios::binary);
    return g_logFile.is_open();
}

void PacketForwarder::LogPacket(const std::string &ts, uint32_t msg_num, uint32_t id_field, double x, double y, double z,double dist, double delay_us)
{
    if (!g_logFile.is_open())
        return;

    char buffer[256];
    int n = snprintf(buffer, sizeof(buffer),
                     "[%s] msg#%u id=0x%08X -> "
                     "x=%.3f, y=%.3f, z=%.3f | "
                     "dist=%.3f m | delay=%.3f us\n",
                     ts.c_str(),
                     msg_num,
                     id_field,
                     x, y, z,
                     dist,
                     delay_us);

    if (n > 0)
        g_logFile.write(buffer, n);   // no flush: VERY FAST
}
void PacketForwarder::LogCSV(const std::string &ts,
                uint32_t msg_num,
                uint32_t id_field,
                double x, double y, double z,
                double Xtp, double Ytp, double Ztp,
                double dist, double delay)
{
    if (!g_logFile.is_open())
        return;

    char buffer[256];

    int n = snprintf(buffer, sizeof(buffer),
                     "%s,%u,0x%08X,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.12f\n",
                     ts.c_str(),
                     msg_num,
                     id_field,
                     x, y, z,
                     Xtp, Ytp, Ztp,
                     dist,
                     delay);   // delay IN SECONDS to match Python

    if (n > 0)
        g_logFile.write(buffer, n);
}
// Simple UTC ISO8601 timestamp like Python's datetime.utcnow().isoformat() + "Z"
std::string PacketForwarder::now_utc_iso8601()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    auto tt  = system_clock::to_time_t(now);
    auto us  = duration_cast<microseconds>(now.time_since_epoch()) % 1000000;

    std::tm tm_utc{};
#ifdef _WIN32
    gmtime_s(&tm_utc, &tt);
#else
    gmtime_r(&tt, &tm_utc);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm_utc, "%Y-%m-%dT%H:%M:%S");
    oss << '.' << std::setw(6) << std::setfill('0') << us.count() << 'Z';
    return oss.str();
}
// ---------------------------------------------------------------------
// initialize() – handles:
//  - Winsock (Windows)
//  - pcap device open
//  - BPF filter
//  - pcap dumper
//  - UDP socket creation
// ---------------------------------------------------------------------
bool PacketForwarder::initialize()
{
    char errbuf[PCAP_ERRBUF_SIZE] = {0};

#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsaData;
    int wsaRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaRes != 0) {
        LOG_ERROR("WSAStartup failed: %d", wsaRes);
        return false;
    }
    m_wsaInitialized = true;
#endif

    // -------------------------------------------------------------
    // 1. Find capture interface
    // -------------------------------------------------------------
    pcap_if_t *alldevs = nullptr;
    if (pcap_findalldevs(&alldevs, errbuf) == -1) {
        LOG_ERROR("pcap_findalldevs failed: %s", errbuf);
        return false;
    }

    pcap_if_t *chosenDev = nullptr;

    if (!m_interfaceName.isEmpty()) {
        for (pcap_if_t *d = alldevs; d != nullptr; d = d->next) {
            if (QString::fromLocal8Bit(d->name) == m_interfaceName) {
                chosenDev = d;
                break;
            }
        }
    }

    // If no match, fall back to first device
    if (!chosenDev) {
        chosenDev = alldevs;
    }

    if (!chosenDev) {
        LOG_ERROR("No capture interfaces found.");
        pcap_freealldevs(alldevs);
        return false;
    }

    {
        std::string name = chosenDev->name ? chosenDev->name : "<null>";
        LOG_INFO("Using capture interface: %s", name.c_str());
    }

    // -------------------------------------------------------------
    // 2. Open device for live capture
    // -------------------------------------------------------------
    m_pcapHandle = pcap_open_live(chosenDev->name,
                                  65535,  // snaplen
                                  1,      // promiscuous
                                  1000,   // timeout (ms)
                                  errbuf);

    pcap_freealldevs(alldevs);
    if (!m_pcapHandle) {
        LOG_ERROR("pcap_open_live failed: %s", errbuf);
        return false;
    }

    // -------------------------------------------------------------
    // 3. Compile & set BPF filter: "udp and dst port <listenPort>"
    // -------------------------------------------------------------
    QString filterStr = QString(
                            "udp and src host %1 and dst host %2 and dst port %3"
                            ).arg("10.0.0.5")
                            .arg("192.168.1.10")
                            .arg(5000);

    if (!applyFilter(filterStr)) {
        LOG_ERROR("Failed to apply filter: %s", filterStr.toStdString().c_str());
        return false;
    }

    // -------------------------------------------------------------
    // 4. Open pcap dumper (full Ethernet frames to file)
    // -------------------------------------------------------------
    QByteArray dumpFileBA = m_dumpFilePath.toLocal8Bit();
    m_pcapDumper = pcap_dump_open(m_pcapHandle, dumpFileBA.constData());
    if (!m_pcapDumper) {
        LOG_ERROR("pcap_dump_open failed: %s", pcap_geterr(m_pcapHandle));
        return false;
    }

    // -------------------------------------------------------------
    // 5. Create UDP socket for forwarding
    // -------------------------------------------------------------
    m_udpSock = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (m_udpSock < 0) {
        LOG_ERROR("socket() failed for UDP forwarder.");
        return false;
    }

    std::string ipStr = m_forwardIp.toStdString();
    LOG_INFO("PacketForwarder config: listen UDP dst=%u, forward=%s:%u, dump=%s", static_cast<unsigned>(m_listenUdpPort), ipStr.c_str(), static_cast<unsigned>(m_forwardUdpPort), m_dumpFilePath.toStdString().c_str());

    return true;
}

void PacketForwarder::captureLoop()
{
    // Pre-build destination address (for forwarding if you still want it)
    sockaddr_in destAddr{};
    destAddr.sin_family = AF_INET;
    destAddr.sin_port   = htons(m_forwardUdpPort);

    std::string ipStr = m_forwardIp.toStdString();

#ifdef _WIN32
    if (InetPtonA(AF_INET, ipStr.c_str(), &destAddr.sin_addr) != 1) {
        LOG_ERROR("InetPtonA failed for IP %s, forwarding may fail.", ipStr.c_str());
    }
#else
    if (::inet_pton(AF_INET, ipStr.c_str(), &destAddr.sin_addr) != 1) {
        LOG_ERROR("inet_pton failed for IP %s, forwarding may fail.", ipStr.c_str());
    }
#endif

    while (!m_stopRequested.load(std::memory_order_relaxed)) {
        struct pcap_pkthdr *header = nullptr;
        const u_char *data = nullptr;

        int res = pcap_next_ex(m_pcapHandle, &header, &data);

        if (res == 0) {
            // timeout; no packet
            // (Python: socket.timeout => "waiting for packet... (timeout)")
            // You can log periodically if you want.
            continue;
        }
        if (res == -1) {
            LOG_ERROR("pcap_next_ex error: %s", pcap_geterr(m_pcapHandle));
            break;
        }
        if (res == -2) {
            LOG_INFO("pcap_next_ex returned EOF (offline capture?).");
            break;
        }

        // 1) Dump full Ethernet frame (like saving raw packets)
        pcap_dump(reinterpret_cast<u_char *>(m_pcapDumper), header, data);

        // 2) Parse Ethernet + IPv4 + UDP to get payload (same as before)
        if (header->caplen < 14 + 20 + 8)
            continue;   // too short

        const u_char *eth = data;
        uint16_t etherType = static_cast<uint16_t>((eth[12] << 8) | eth[13]);

        // Only IPv4
        if (etherType != 0x0800)
            continue;

        const u_char *ip = eth + 14;
        uint8_t versionIhl = ip[0];
        uint8_t ihl = versionIhl & 0x0F;
        int ipHeaderLen = ihl * 4;

        if (header->caplen < 14 + ipHeaderLen + 8)
            continue;

        uint8_t protocol = ip[9];
        if (protocol != 17)   // UDP = 17
            continue;

        const u_char *udp = ip + ipHeaderLen;

        uint16_t dstPort = static_cast<uint16_t>((udp[2] << 8) | udp[3]);
        uint16_t udpLen  = static_cast<uint16_t>((udp[4] << 8) | udp[5]);

        if (dstPort != m_listenUdpPort)
            continue;

        if (udpLen <= 8)
            continue;

        int payloadLen = udpLen - 8;
        const char *payload = reinterpret_cast<const char *>(udp + 8);

        if (header->caplen < 14 + ipHeaderLen + 8 + payloadLen)
            continue;

        // -----------------------------------------------------------------
        // 3) Now replicate the Python UDP client logic on THIS payload
        // -----------------------------------------------------------------

        if (payloadLen < static_cast<int>(STRUCT_SIZE)) {
            LOG_ERROR("[CLIENT] Received packet too small: %d bytes", payloadLen);
            continue;
        }

        UdpPayload up{};
        std::memcpy(&up, payload, STRUCT_SIZE);

        // If your struct is in network byte order, convert integers:
        // up.id_field = ntohl(up.id_field);
        // up.msg_num  = ntohl(up.msg_num);
        // up.reserved = ntohl(up.reserved);

        double x = up.x;
        double y = up.y;
        double z = up.z;

        double dist = 0.0;
        double delay = 0.0;
        compute_delay(Xtp, Ytp, Ztp, x, y, z, dist, delay);

        std::string ts = now_utc_iso8601();

        // Log similar to Python:
        // f"[{ts}] msg#{msg_num} id=0x{id_field:08X} -> x=... | dist=... | delay=... µs"
        LOG_INFO("[%s] msg#%u id=0x%08X -> x=%.3f, y=%.3f, z=%.3f | dist=%.3f m | delay=%.3f us",ts.c_str(),up.msg_num,up.id_field,x, y, z,dist,delay * 1e6);

        // -----------------------------------------------------------------
        // 4) Forward UDP payload (if you still want to relay it)
        // -----------------------------------------------------------------
        int sent = ::sendto(m_udpSock,
                            payload,
                            payloadLen,
                            0,
                            reinterpret_cast<sockaddr *>(&destAddr),
                            sizeof(destAddr));

        if (sent != payloadLen) {
            LOG_ERROR("sendto() failed or partial send. Sent=%d, expected=%d",
                      sent, payloadLen);
        }

        // CSV file writing is intentionally omitted (as you requested).
    }
}
#if 0
// ---------------------------------------------------------------------
// captureLoop() – continuous:
//   - pcap_next_ex()
//   - dump full Ethernet frame to .pcap
//   - parse Ethernet + IPv4 + UDP
//   - send UDP payload via socket
// ---------------------------------------------------------------------
void PacketForwarder::captureLoop()
{
    // Pre-build destination address
    sockaddr_in destAddr{};
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(m_forwardUdpPort);

    std::string ipStr = m_forwardIp.toStdString();

#ifdef _WIN32
    if (InetPtonA(AF_INET, ipStr.c_str(), &destAddr.sin_addr) != 1) {
        LOG_ERROR("InetPtonA failed for IP %s, forwarding may fail.", ipStr.c_str());
    }
#else
    if (::inet_pton(AF_INET, ipStr.c_str(), &destAddr.sin_addr) != 1) {
        LOG_ERROR("inet_pton failed for IP %s, forwarding may fail.", ipStr.c_str());
    }
#endif

    while (!m_stopRequested.load(std::memory_order_relaxed)) {
        struct pcap_pkthdr *header = nullptr;
        const u_char *data = nullptr;

        int res = pcap_next_ex(m_pcapHandle, &header, &data);

        if (res == 0) {
            // timeout; no packet
            continue;
        }
        if (res == -1) {
            LOG_ERROR("pcap_next_ex error: %s", pcap_geterr(m_pcapHandle));
            break;
        }
        if (res == -2) {
            LOG_INFO("pcap_next_ex returned EOF (offline capture?).");
            break;
        }

        // 1) Dump full Ethernet frame
        pcap_dump(reinterpret_cast<u_char *>(m_pcapDumper), header, data);

        // 2) Parse Ethernet + IPv4 + UDP to get payload
        if (header->caplen < 14 + 20 + 8)
            continue;   // too short

        const u_char *eth = data;
        uint16_t etherType = static_cast<uint16_t>((eth[12] << 8) | eth[13]);

        // Only IPv4
        if (etherType != 0x0800)
            continue;

        const u_char *ip = eth + 14;
        uint8_t versionIhl = ip[0];
        uint8_t ihl = versionIhl & 0x0F;
        int ipHeaderLen = ihl * 4;

        if (header->caplen < 14 + ipHeaderLen + 8)
            continue;

        uint8_t protocol = ip[9];
        if (protocol != 17)   // UDP = 17
            continue;

        const u_char *udp = ip + ipHeaderLen;

        uint16_t dstPort = static_cast<uint16_t>((udp[2] << 8) | udp[3]);
        uint16_t udpLen  = static_cast<uint16_t>((udp[4] << 8) | udp[5]);

        if (dstPort != m_listenUdpPort)
            continue;

        if (udpLen <= 8)
            continue;

        int payloadLen = udpLen - 8;
        const char *payload = reinterpret_cast<const char *>(udp + 8);

        if (header->caplen < 14 + ipHeaderLen + 8 + payloadLen)
            continue;

        // 3) Forward UDP payload via socket
        int sent = ::sendto(m_udpSock, payload, payloadLen, 0, reinterpret_cast<sockaddr *>(&destAddr), sizeof(destAddr));
        if (sent != payloadLen) {
            LOG_ERROR("sendto() failed or partial send. Sent=%d, expected=%d",
                      sent, payloadLen);
        }
    }
}
#endif

double PacketForwarder::random_uniform(double a, double b)
{
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> dist(a, b);
    return dist(rng);
}

Position PacketForwarder::generate_position(double t)
{
    Position p;

    // Synthetic circular motion + noise
    p.x = 100.0 * std::cos(0.5 * t) + 0.5 * random_uniform(-1.0, 1.0);
    p.y = 100.0 * std::sin(0.5 * t) + 0.5 * random_uniform(-1.0, 1.0);
    p.z = 50.0 + 0.1 * t + 0.2 * random_uniform(-1.0, 1.0);

    return p;
}

void PacketForwarder::compute_delay(double Xtp, double Ytp, double Ztp,
                   double x, double y, double z,
                   double &dist, double &delay)
{
    double dx = Xtp - x;
    double dy = Ytp - y;
    double dz = Ztp - z;

    dist = std::sqrt(dx*dx + dy*dy + dz*dz);

    // round trip delay
    delay = 2.0 * dist / SPEED_OF_LIGHT;
}
// ---------------------------------------------------------------------
// cleanup() – release all resources
// ---------------------------------------------------------------------
void PacketForwarder::cleanup()
{
    if (m_pcapDumper) {
        pcap_dump_close(m_pcapDumper);
        m_pcapDumper = nullptr;
    }

    if (m_pcapHandle) {
        pcap_close(m_pcapHandle);
        m_pcapHandle = nullptr;
    }

    if (m_udpSock >= 0) {
#ifdef _WIN32
        ::closesocket(m_udpSock);
#else
        ::close(m_udpSock);
#endif
        m_udpSock = -1;
    }

#ifdef _WIN32
    if (m_wsaInitialized) {
        WSACleanup();
        m_wsaInitialized = false;
    }
#endif
}
