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
    QString filterStr = QString("udp and dst port %1").arg(m_listenUdpPort);
    QByteArray filterBA = filterStr.toLocal8Bit();

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
