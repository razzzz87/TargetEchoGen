#include "packetforwarder.h"
#include "log.h"

#include <cstdint>
#include <cstring>
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
#include <errno.h>
#endif

static constexpr size_t STRUCT_SIZE = sizeof(UdpPayload);

// ---------------------------------------------------------------------
// Small helper for ISO8601 timestamps
// ---------------------------------------------------------------------
static std::string utc_iso8601_now()
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

// =====================================================================
// PacketForwarder implementation
// =====================================================================

PacketForwarder::PacketForwarder(const QString &srcIp,
                                 quint16 srcPort,
                                 const QString &dstIp,
                                 quint16 dstPort,
                                 const QString &csvPath,
                                 QObject *parent)
    : QThread(parent),
    m_srcIp(srcIp),
    m_srcPort(srcPort),
    m_dstIp(dstIp),
    m_dstPort(dstPort),
    m_csvPath(csvPath),
    m_stopRequested(false),
    m_srcSock(-1),
    m_dstSock(-1)
#ifdef _WIN32
    , m_wsaInitialized(false)
#endif
{
}

PacketForwarder::~PacketForwarder()
{
    stop();
    wait();
}

void PacketForwarder::stop()
{
    m_stopRequested.store(true, std::memory_order_relaxed);
}

void PacketForwarder::setTarget(double Xtp, double Ytp, double Ztp)
{
    m_Xtp = Xtp;
    m_Ytp = Ytp;
    m_Ztp = Ztp;
}

// ---------------------------------------------------------------------
// CSV helpers
// ---------------------------------------------------------------------
bool PacketForwarder::openCsv(const std::string &path)
{
    bool first = false;

    {
        std::ifstream test(path, std::ios::in | std::ios::binary);
        first = !test.good();
    }

    m_csv.open(path, std::ios::out | std::ios::app);
    if (!m_csv.is_open()) {
        LOG_ERROR("PacketForwarder: Failed to open CSV file: %s", path.c_str());
        return false;
    }

    if (first) {
        m_csv << "timestamp_utc,msg_number,id_hex,"
              << "x,y,z,Xtp,Ytp,Ztp,distance_m,delay_s\n";
        m_csv.flush();
    }

    return true;
}

void PacketForwarder::writeCsvRow(const std::string &ts,
                                  uint32_t msg_num,
                                  uint32_t id_field,
                                  double x, double y, double z,
                                  double dist, double delay_s)
{
    if (!m_csv.is_open())
        return;

    char id_hex[16];
    std::snprintf(id_hex, sizeof(id_hex), "0x%08X", id_field);

    m_csv << ts << ","
          << msg_num << ","
          << id_hex << ","
          << std::fixed << std::setprecision(6)
          << x << "," << y << "," << z << ","
          << m_Xtp << "," << m_Ytp << "," << m_Ztp << ","
          << dist << ","
          << std::setprecision(12) << delay_s
          << "\n";

    m_csv.flush();
}

// ---------------------------------------------------------------------
// Time + delay
// ---------------------------------------------------------------------
std::string PacketForwarder::now_utc_iso8601()
{
    return utc_iso8601_now();
}

void PacketForwarder::compute_delay(double Xtp, double Ytp, double Ztp,
                                    double x, double y, double z,
                                    double &dist, double &delay)
{
    double dx = Xtp - x;
    double dy = Ytp - y;
    double dz = Ztp - z;

    dist = std::sqrt(dx*dx + dy*dy + dz*dz);
    delay = 2.0 * dist / SPEED_OF_LIGHT;
}

// ---------------------------------------------------------------------
// Thread entry
// ---------------------------------------------------------------------
void PacketForwarder::run()
{
    if (!initialize()) {
        LOG_ERROR("PacketForwarder: initialization failed, exiting thread.");
        cleanup();
        return;
    }

    if (!m_csvPath.isEmpty()) {
        openCsv(m_csvPath.toStdString());
    }

    LOG_INFO("PacketForwarder: initialization OK, entering relay loop.");
    relayLoop();
    cleanup();
    LOG_INFO("PacketForwarder: thread exited.");
}

// ---------------------------------------------------------------------
// initialize() – create two client sockets:
//  - m_srcSock: talk to IMS server (send READY, recv)
//  - m_dstSock: forward packets to other server
// ---------------------------------------------------------------------
bool PacketForwarder::initialize()
{
#ifdef _WIN32
    WSADATA wsaData;
    int wsaRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaRes != 0) {
        LOG_ERROR("PacketForwarder: WSAStartup failed: %d", wsaRes);
        return false;
    }
    m_wsaInitialized = true;
#endif

    // 1) SRC socket: this is the "client" to the IMS server (server.py style)
    m_srcSock = ::socket(AF_INET, SOCK_DGRAM, 0);
#ifdef _WIN32
    if (m_srcSock == INVALID_SOCKET) {
        LOG_ERROR("PacketForwarder: src socket() failed: %d", WSAGetLastError());
        return false;
    }
#else
    if (m_srcSock < 0) {
        LOG_ERROR("PacketForwarder: src socket() failed: %s", strerror(errno));
        return false;
    }
#endif

    // Bind to local ephemeral port (like Python LOCAL_BIND = ('',0))
    sockaddr_in localAddr{};
    localAddr.sin_family = AF_INET;
    localAddr.sin_port   = htons(0);  // 0 => ephemeral

    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (::bind(m_srcSock,
               reinterpret_cast<sockaddr*>(&localAddr),
               sizeof(localAddr)) < 0) {
#ifdef _WIN32
        LOG_ERROR("PacketForwarder: bind(srcSock) failed: %d", WSAGetLastError());
#else
        LOG_ERROR("PacketForwarder: bind(srcSock) failed: %s", strerror(errno));
#endif
        return false;
    }

// Optional timeout on recv
#ifdef _WIN32
    {
        DWORD timeoutMs = 2000;
        setsockopt(m_srcSock, SOL_SOCKET, SO_RCVTIMEO,
                   reinterpret_cast<const char*>(&timeoutMs), sizeof(timeoutMs));
    }
#else
    {
        timeval tv{};
        tv.tv_sec  = 2;
        tv.tv_usec = 0;
        setsockopt(m_srcSock, SOL_SOCKET, SO_RCVTIMEO,
                   &tv, sizeof(tv));
    }
#endif

    // Build IMS server address
    sockaddr_in srcServer{};
    srcServer.sin_family = AF_INET;
    srcServer.sin_port   = htons(m_srcPort);

    std::string srcIpStr = m_srcIp.toStdString();
#ifdef _WIN32
    if (InetPtonA(AF_INET, srcIpStr.c_str(), &srcServer.sin_addr) != 1) {
        LOG_ERROR("PacketForwarder: InetPtonA failed for src IP %s", srcIpStr.c_str());
        return false;
    }
#else
    if (::inet_pton(AF_INET, srcIpStr.c_str(), &srcServer.sin_addr) != 1) {
        LOG_ERROR("PacketForwarder: inet_pton failed for src IP %s", srcIpStr.c_str());
        return false;
    }
#endif

    // Print local address
    {
        sockaddr_in actualLocal{};
        socklen_t   len = sizeof(actualLocal);
        if (getsockname(m_srcSock, reinterpret_cast<sockaddr*>(&actualLocal), &len) == 0) {
            char addrBuf[64];
#ifdef _WIN32
            InetNtopA(AF_INET, &actualLocal.sin_addr, addrBuf, sizeof(addrBuf));
#else
            inet_ntop(AF_INET, &actualLocal.sin_addr, addrBuf, sizeof(addrBuf));
#endif
            LOG_INFO("[CLIENT] srcSock bound to %s:%u, sending READY to %s:%u",
                     addrBuf,
                     ntohs(actualLocal.sin_port),
                     srcIpStr.c_str(),
                     static_cast<unsigned>(m_srcPort));
        }
    }

    // Send READY to IMS server (like Python client)
    {
        const char readyPayload[] = "READY";
        int sent = ::sendto(m_srcSock,
                            readyPayload,
                            sizeof(readyPayload) - 1,
                            0,
                            reinterpret_cast<sockaddr*>(&srcServer),
                            sizeof(srcServer));
        if (sent <= 0) {
            LOG_ERROR("[CLIENT] Failed to send READY to IMS server");
            return false;
        }
    }

    // 2) DST socket: used only to forward packets to another server
    m_dstSock = ::socket(AF_INET, SOCK_DGRAM, 0);
#ifdef _WIN32
    if (m_dstSock == INVALID_SOCKET) {
        LOG_ERROR("PacketForwarder: dst socket() failed: %d", WSAGetLastError());
        return false;
    }
#else
    if (m_dstSock < 0) {
        LOG_ERROR("PacketForwarder: dst socket() failed: %s", strerror(errno));
        return false;
    }
#endif

    // That's it; we build dst address on every send in relayLoop.
    LOG_INFO("PacketForwarder SRC server=%s:%u, DST server=%s:%u",
             srcIpStr.c_str(),
             static_cast<unsigned>(m_srcPort),
             m_dstIp.toStdString().c_str(),
             static_cast<unsigned>(m_dstPort));

    return true;
}

// ---------------------------------------------------------------------
// relayLoop() – recv from m_srcSock, process, forward via m_dstSock
// ---------------------------------------------------------------------
void PacketForwarder::relayLoop()
{
    char buf[2048];

    // Pre-build destination address for forwarding
    sockaddr_in dstAddr{};
    dstAddr.sin_family = AF_INET;
    dstAddr.sin_port   = htons(m_dstPort);

    std::string dstIpStr = m_dstIp.toStdString();
#ifdef _WIN32
    if (InetPtonA(AF_INET, dstIpStr.c_str(), &dstAddr.sin_addr) != 1) {
        LOG_ERROR("PacketForwarder: InetPtonA failed for dst IP %s", dstIpStr.c_str());
        // we still run, but forwarding will fail
    }
#else
    if (::inet_pton(AF_INET, dstIpStr.c_str(), &dstAddr.sin_addr) != 1) {
        LOG_ERROR("PacketForwarder: inet_pton failed for dst IP %s", dstIpStr.c_str());
    }
#endif

    while (!m_stopRequested.load(std::memory_order_relaxed)) {
        sockaddr_in srcAddr{};
        socklen_t   addrLen = sizeof(srcAddr);

        int n = ::recvfrom(m_srcSock,
                           buf,
                           static_cast<int>(sizeof(buf)),
                           0,
                           reinterpret_cast<sockaddr*>(&srcAddr),
                           &addrLen);
        if (n < 0) {
#ifdef _WIN32
            int err = WSAGetLastError();
            if (err == WSAETIMEDOUT) {
                LOG_INFO("[CLIENT] waiting for packet... (timeout)");
                continue;
            }
            if (err == WSAEINTR)
                continue;
            LOG_ERROR("PacketForwarder: recvfrom() failed: %d", err);
#else
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                LOG_INFO("[CLIENT] waiting for packet... (timeout)");
                continue;
            }
            if (errno == EINTR)
                continue;
            LOG_ERROR("PacketForwarder: recvfrom() failed: %s", strerror(errno));
#endif
            break;
        }
        if (n == 0)
            continue;

        if (n < static_cast<int>(STRUCT_SIZE)) {
            LOG_ERROR("[CLIENT] Received packet too small: %d bytes", n);
            continue;
        }

        UdpPayload up{};
        std::memcpy(&up, buf, STRUCT_SIZE);

        // If server sends big-endian ints, convert here (depends on server impl)
        // up.id_field = ntohl(up.id_field);
        // up.msg_num  = ntohl(up.msg_num);

        double x = up.x;
        double y = up.y;
        double z = up.z;

        double dist  = 0.0;
        double delay = 0.0;
        compute_delay(m_Xtp, m_Ytp, m_Ztp, x, y, z, dist, delay);

        std::string ts = now_utc_iso8601();

        LOG_INFO("[%s] msg#%u id=0x%08X -> x=%.3f, y=%.3f, z=%.3f | "
                 "dist=%.3f m | delay=%.3f us",
                 ts.c_str(),
                 up.msg_num,
                 up.id_field,
                 x, y, z,
                 dist,
                 delay * 1e6);

        writeCsvRow(ts, up.msg_num, up.id_field, x, y, z, dist, delay);

        // Forward the *same* payload to other server via m_dstSock
        if (m_dstPort != 0 && !dstIpStr.empty()) {
            int sent = ::sendto(m_dstSock,
                                buf,
                                n,
                                0,
                                reinterpret_cast<sockaddr*>(&dstAddr),
                                sizeof(dstAddr));
            if (sent != n) {
                LOG_ERROR("PacketForwarder: sendto(dst) failed or partial send. Sent=%d, expected=%d",
                          sent, n);
            }
        }
    }
}

// ---------------------------------------------------------------------
// cleanup()
// ---------------------------------------------------------------------
void PacketForwarder::cleanup()
{
    if (m_srcSock >= 0) {
#ifdef _WIN32
        ::closesocket(m_srcSock);
#else
        ::close(m_srcSock);
#endif
        m_srcSock = -1;
    }

    if (m_dstSock >= 0) {
#ifdef _WIN32
        ::closesocket(m_dstSock);
#else
        ::close(m_dstSock);
#endif
        m_dstSock = -1;
    }

#ifdef _WIN32
    if (m_wsaInitialized) {
        WSACleanup();
        m_wsaInitialized = false;
    }
#endif

    if (m_csv.is_open())
        m_csv.close();
}

// auto relay = new PacketForwarder(
//     "127.0.0.1",      // src server IP (IMS server / server.py)
//     0xD407,           // src server port
//     "192.168.1.50",   // dst server IP (where you forward)
//     6000,             // dst server port
//     "relay_log.csv",  // CSV log
//     this
//     );

// relay->setTarget(10.0, 10.0, 10.0);   // Xtp, Ytp, Ztp
// relay->start();
