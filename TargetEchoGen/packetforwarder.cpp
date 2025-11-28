#include "packetforwarder.h"
#include "Utils.h"
#include "log.h"
#include "proto.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <QFileInfo>
#include <QFile>

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

// 10 MB limit
static const qint64 MAX_CSV_SIZE = 10LL * 1024LL * 1024LL;

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
                                 const QString &txIp,
                                 quint16 txPort,
                                 const QString &csvPath,
                                 QObject *parent)
    : QThread(parent),
    m_srcIp(srcIp),
    m_srcPort(srcPort),
    m_txIp(txIp),
    m_txPort(txPort),
    m_csvPath(csvPath),
    m_stopRequested(false),
    m_udpRecvSock(-1),
    m_udpTxSock(-1),
    m_Xtp(10.0),
    m_Ytp(10.0),
    m_Ztp(10.0),
    m_startTime(0.0)
#ifdef _WIN32
    , m_wsaInitialized(false)
#endif
{
    LOG_INFO("PacketForwarder constructed (UDP recv + UDP tx)");
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

void PacketForwarder::setTargetCoordinates(double x, double y, double z)
{
    m_Xtp = x;
    m_Ytp = y;
    m_Ztp = z;
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

void PacketForwarder::rotateCsvIfNeeded()
{
    if (m_csvPath.isEmpty())
        return;

    QFileInfo fi(m_csvPath);
    if (!fi.exists())
        return;

    if (fi.size() < MAX_CSV_SIZE)
        return;    // below limit, no rotation

    if (m_csv.is_open()) {
        m_csv.close();
    }

    QString backupPath = m_csvPath + ".bak";
    QFile::remove(backupPath);

    if (!QFile::rename(m_csvPath, backupPath)) {
        LOG_ERROR("PacketForwarder: failed to rotate CSV. Rename %s -> %s failed",
                  m_csvPath.toStdString().c_str(),
                  backupPath.toStdString().c_str());
        return;
    }

    openCsv(m_csvPath.toStdString());
}

// ---------------------------------------------------------------------
// Synthetic motion + delay
// ---------------------------------------------------------------------
double PacketForwarder::rand_uniform()
{
    return 2.0 * (double(rand()) / RAND_MAX) - 1.0;
}

Position PacketForwarder::generate_position(double t)
{
    Position p;
    p.x = 100.0 * std::cos(0.5 * t) + 0.5 * rand_uniform();
    p.y = 100.0 * std::sin(0.5 * t) + 0.5 * rand_uniform();
    p.z = 50.0 + 0.1 * t + 0.2 * rand_uniform();
    return p;
}

void PacketForwarder::compute_delay(double Xtp, double Ytp, double Ztp,
                                    double x,   double y,   double z,
                                    double &dist_m, int &delay_us)
{
    double dx = Xtp - x;
    double dy = Ytp - y;
    double dz = Ztp - z;

    dist_m = std::sqrt(dx*dx + dy*dy + dz*dz);

    double delay_sec  = 2.0 * dist_m / SPEED_OF_LIGHT;
    double delay_us_f = delay_sec * 1e6;

    if (delay_us_f <= 0.0) {
        delay_us = 0;
    } else if (delay_us_f < 1.0) {
        delay_us = 1;
    } else {
        delay_us = static_cast<int>(std::lround(delay_us_f));
    }
}

// ---------------------------------------------------------------------
// CSV row writer
// ---------------------------------------------------------------------
void PacketForwarder::writeCsvRow(const std::string &ts,
                                  uint32_t msg_num,
                                  uint32_t id_field,
                                  double x, double y, double z,
                                  double dist,
                                  double delay_s)
{
    if (!m_csv.is_open())
        return;

    rotateCsvIfNeeded();

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
// Time helper
// ---------------------------------------------------------------------
std::string PacketForwarder::now_utc_iso8601()
{
    return utc_iso8601_now();
}

// ---------------------------------------------------------------------
// Thread entry
// ---------------------------------------------------------------------
void PacketForwarder::run()
{
    if (m_udpRecvSock < 0) {
        LOG_ERROR("PacketForwarder: run() called without initialization!");
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
// initialize() – create sockets
//  - m_udpRecvSock: UDP receive (bind to srcPort)
//  - m_udpTxSock:   UDP transmit (delay + file)
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

    // 1) UDP RECEIVE SOCKET
    m_udpRecvSock = ::socket(AF_INET, SOCK_DGRAM, 0);
#ifdef _WIN32
    if (m_udpRecvSock == INVALID_SOCKET) {
        LOG_ERROR("PacketForwarder: udpRecv socket() failed: %d", WSAGetLastError());
        return false;
    }
#else
    if (m_udpRecvSock < 0) {
        LOG_ERROR("PacketForwarder: udpRecv socket() failed: %s", strerror(errno));
        return false;
    }
#endif

    {
        sockaddr_in localAddr{};
        localAddr.sin_family      = AF_INET;
        localAddr.sin_port        = htons(m_srcPort);   // bind to given port
        localAddr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (::bind(m_udpRecvSock,
                   reinterpret_cast<sockaddr*>(&localAddr),
                   sizeof(localAddr)) < 0) {
#ifdef _WIN32
            LOG_ERROR("PacketForwarder: bind(udpRecv) failed: %d", WSAGetLastError());
#else
            LOG_ERROR("PacketForwarder: bind(udpRecv) failed: %s", strerror(errno));
#endif
            return false;
        }
    }

#ifdef _WIN32
    {
        DWORD timeoutMs = 2000;
        setsockopt(m_udpRecvSock, SOL_SOCKET, SO_RCVTIMEO,
                   reinterpret_cast<const char*>(&timeoutMs),
                   sizeof(timeoutMs));
    }
#else
    {
        timeval tv{};
        tv.tv_sec  = 2;
        tv.tv_usec = 0;
        setsockopt(m_udpRecvSock, SOL_SOCKET, SO_RCVTIMEO,
                   &tv, sizeof(tv));
    }
#endif

    {
        sockaddr_in actualLocal{};
        socklen_t   len = sizeof(actualLocal);
        if (getsockname(m_udpRecvSock,
                        reinterpret_cast<sockaddr*>(&actualLocal),
                        &len) == 0) {
            char addrBuf[64];
#ifdef _WIN32
            InetNtopA(AF_INET, &actualLocal.sin_addr, addrBuf, sizeof(addrBuf));
#else
            inet_ntop(AF_INET, &actualLocal.sin_addr, addrBuf, sizeof(addrBuf));
#endif
            LOG_INFO("[CLIENT] UDP recv socket bound to %s:%u",
                     addrBuf,
                     ntohs(actualLocal.sin_port));
        }
    }

    // 2) UDP TX SOCKET (used for delay + file packets)
    m_udpTxSock = ::socket(AF_INET, SOCK_DGRAM, 0);
#ifdef _WIN32
    if (m_udpTxSock == INVALID_SOCKET) {
        LOG_ERROR("PacketForwarder: udpTx socket() failed: %d", WSAGetLastError());
        return false;
    }
#else
    if (m_udpTxSock < 0) {
        LOG_ERROR("PacketForwarder: udpTx socket() failed: %s", strerror(errno));
        return false;
    }
#endif

    // capture start time for synthetic motion used in send_delay_once()
    m_startTime = std::chrono::duration<double>(
                      std::chrono::system_clock::now().time_since_epoch()
                      ).count();

    LOG_INFO("PacketForwarder: INIT OK – UDP recv + UDP tx sockets created.");
    return true;
}

// ---------------------------------------------------------------------
// relayLoop() – UDP recv -> compute delay/log
// ---------------------------------------------------------------------
void PacketForwarder::relayLoop()
{
    char buf[2048];

    while (!m_stopRequested.load(std::memory_order_relaxed)) {
        sockaddr_in srcAddr{};
        socklen_t   addrLen = sizeof(srcAddr);

        int n = ::recvfrom(
            m_udpRecvSock,
            buf,
            static_cast<int>(sizeof(buf)),
            0,
            reinterpret_cast<sockaddr*>(&srcAddr),
            &addrLen
            );

        if (n < 0) {
#ifdef _WIN32
            int err = WSAGetLastError();
            if (err == WSAETIMEDOUT) {
                LOG_INFO("[CLIENT] waiting for packet... (timeout)");
                continue;
            }
            if (err == WSAEINTR)
                continue;
            LOG_ERROR("PacketForwarder: recvfrom(udpRecv) failed: %d", err);
#else
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                LOG_INFO("[CLIENT] waiting for packet... (timeout)");
                continue;
            }
            if (errno == EINTR)
                continue;
            LOG_ERROR("PacketForwarder: recvfrom(udpRecv) failed: %s", strerror(errno));
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

        double x = up.x;
        double y = up.y;
        double z = up.z;

        double dist_m = 0.0;
        int    delay_us = 0;
        compute_delay(m_Xtp, m_Ytp, m_Ztp, x, y, z, dist_m, delay_us);

        double delay_s = static_cast<double>(delay_us) * 1e-6;
        std::string ts = now_utc_iso8601();

        LOG_INFO("relayLoop: Pos(%.3f, %.3f, %.3f) -> TP(%.3f, %.3f, %.3f) | dist=%.3f m | delay_us=%d",
                 x, y, z,
                 m_Xtp, m_Ytp, m_Ztp,
                 dist_m,
                 delay_us);

        writeCsvRow(ts, up.msg_num, up.id_field,
                    x, y, z,
                    dist_m,
                    delay_s);

        // NOTE: we do NOT call send_delay_once() automatically here.
        // You call send_delay_once() from outside (e.g. timer / button).
    }
}

// ---------------------------------------------------------------------
// cleanup()
// ---------------------------------------------------------------------
void PacketForwarder::cleanup()
{
    if (m_udpRecvSock >= 0) {
#ifdef _WIN32
        ::closesocket(m_udpRecvSock);
#else
        ::close(m_udpRecvSock);
#endif
        m_udpRecvSock = -1;
    }

    if (m_udpTxSock >= 0) {
#ifdef _WIN32
        ::closesocket(m_udpTxSock);
#else
        ::close(m_udpTxSock);
#endif
        m_udpTxSock = -1;
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

// ---------------------------------------------------------------------
// UDP file send (coordinate/file) using m_udpTxSock
// ---------------------------------------------------------------------
void PacketForwarder::sendCoordinateFileUdp(const QString &filePath)
{
    if (m_udpTxSock < 0) {
        LOG_ERROR("sendCoordinateFileUdp: m_udpTxSock not initialized (call initialize() first)");
        return;
    }

    if (m_txIp.isEmpty() || m_txPort == 0) {
        LOG_ERROR("sendCoordinateFileUdp: invalid UDP TX destination IP/port");
        return;
    }

    QFile file(filePath);
    if (!file.exists()) {
        LOG_ERROR("sendCoordinateFileUdp: file does not exist: %s",
                  filePath.toStdString().c_str());
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR("sendCoordinateFileUdp: failed to open file: %s (error: %s)",
                  filePath.toStdString().c_str(),
                  file.errorString().toStdString().c_str());
        return;
    }

    sockaddr_in txAddr{};
    txAddr.sin_family = AF_INET;
    txAddr.sin_port   = htons(m_txPort);

    std::string txIpStr = m_txIp.toStdString();
#ifdef _WIN32
    if (InetPtonA(AF_INET, txIpStr.c_str(), &txAddr.sin_addr) != 1) {
        LOG_ERROR("sendCoordinateFileUdp: InetPtonA failed for TX IP %s",
                  txIpStr.c_str());
        return;
    }
#else
    if (::inet_pton(AF_INET, txIpStr.c_str(), &txAddr.sin_addr) != 1) {
        LOG_ERROR("sendCoordinateFileUdp: inet_pton failed for TX IP %s",
                  txIpStr.c_str());
        return;
    }
#endif

    LOG_INFO("sendCoordinateFileUdp: sending file %s to %s:%u over UDP",
             filePath.toStdString().c_str(),
             txIpStr.c_str(),
             static_cast<unsigned>(m_txPort));

    const int MAX_UDP_PAYLOAD = 1400;
    QByteArray buf;

    qint64 totalSent = 0;
    int    packetIdx = 0;

    while (true) {
        buf = file.read(MAX_UDP_PAYLOAD);
        if (buf.isEmpty()) {
            if (file.atEnd()) {
                break;
            } else {
                LOG_ERROR("sendCoordinateFileUdp: file read error on %s",
                          filePath.toStdString().c_str());
                break;
            }
        }

        int len = static_cast<int>(buf.size());
        int sent = ::sendto(
            m_udpTxSock,
            buf.constData(),
            len,
            0,
            reinterpret_cast<sockaddr*>(&txAddr),
            sizeof(txAddr)
            );

        if (sent != len) {
#ifdef _WIN32
            int wsaErr = WSAGetLastError();
            if (sent == SOCKET_ERROR) {
                LOG_ERROR("sendCoordinateFileUdp: sendto() FAILED on packet %d, WSAError=%d, expected=%d",
                          packetIdx, wsaErr, len);
                break;
            } else {
                LOG_ERROR("sendCoordinateFileUdp: partial send on packet %d, Sent=%d, expected=%d, WSAError=%d",
                          packetIdx, sent, len, wsaErr);
            }
#else
            int err = errno;
            if (sent < 0) {
                LOG_ERROR("sendCoordinateFileUdp: sendto() FAILED on packet %d, errno=%d (%s), expected=%d",
                          packetIdx, err, strerror(err), len);
                break;
            } else {
                LOG_ERROR("sendCoordinateFileUdp: partial send on packet %d, Sent=%d, expected=%d, errno=%d (%s)",
                          packetIdx, sent, len, err, strerror(err));
            }
#endif
        } else {
            totalSent += sent;
            packetIdx++;
        }
    }

    LOG_INFO("sendCoordinateFileUdp: finished sending %d packets, total %lld bytes",
             packetIdx,
             static_cast<long long>(totalSent));
}

// ---------------------------------------------------------------------
// send_delay_once() – synthetic motion -> delay_us -> UDP send via m_udpTxSock
// ---------------------------------------------------------------------
void PacketForwarder::send_delay_once(double Xtp, double Ytp, double Ztp)
{
    if (m_udpTxSock < 0) {
        LOG_ERROR("PacketForwarder::send_delay_once: m_udpTxSock not initialized (call initialize() first)");
        return;
    }

    if (m_txIp.isEmpty() || m_txPort == 0) {
        LOG_ERROR("PacketForwarder::send_delay_once: invalid TX destination IP/port");
        return;
    }

    double now = std::chrono::duration<double>(
                     std::chrono::system_clock::now().time_since_epoch()
                     ).count();
    double t = now - m_startTime;

    Position p = generate_position(t);

    double dist_m = 0.0;
    int    delay_us = 0;
    compute_delay(Xtp, Ytp, Ztp, p.x, p.y, p.z, dist_m, delay_us);

    LOG_INFO("send_delay_once: m_startTime=%.6f, now=%.6f, t=%.6f | Pos(%.3f, %.3f, %.3f) -> TP(%.3f, %.3f, %.3f) | dist=%.3f m | delay_us=%d",
             m_startTime,
             now,
             t,
             p.x, p.y, p.z,
             Xtp, Ytp, Ztp,
             dist_m,
             delay_us);

    Proto ProtoObj;
    char* byArrPkt = nullptr;
    uint  iaddr    = 0x206C;  // example register for delay

    int pktLen = ProtoObj.mPktRegWrite(iaddr, delay_us, &byArrPkt);
    if (pktLen <= 0 || byArrPkt == nullptr) {
        LOG_ERROR("PacketForwarder::send_delay_once: mPktRegWrite failed, pktLen=%d, buf=%p",
                  pktLen, static_cast<void*>(byArrPkt));
        return;
    }

    sockaddr_in txAddr{};
    txAddr.sin_family = AF_INET;
    txAddr.sin_port   = htons(m_txPort);

    std::string txIpStr = m_txIp.toStdString();
#ifdef _WIN32
    if (InetPtonA(AF_INET, txIpStr.c_str(), &txAddr.sin_addr) != 1) {
        LOG_ERROR("PacketForwarder::send_delay_once: InetPtonA failed for TX IP %s",
                  txIpStr.c_str());
        return;
    }
#else
    if (::inet_pton(AF_INET, txIpStr.c_str(), &txAddr.sin_addr) != 1) {
        LOG_ERROR("PacketForwarder::send_delay_once: inet_pton failed for TX IP %s",
                  txIpStr.c_str());
        return;
    }
#endif

    int sent = ::sendto(
        m_udpTxSock,
        byArrPkt,
        pktLen,
        0,
        reinterpret_cast<sockaddr*>(&txAddr),
        sizeof(txAddr)
        );

    if (sent != pktLen) {
#ifdef _WIN32
        int wsaErr = WSAGetLastError();
        if (sent == SOCKET_ERROR) {
            LOG_ERROR("PacketForwarder::send_delay_once: sendto() FAILED, WSAError=%d, expected=%d",
                      wsaErr, pktLen);
        } else {
            LOG_ERROR("PacketForwarder::send_delay_once: partial send, Sent=%d, expected=%d, WSAError=%d",
                      sent, pktLen, wsaErr);
        }
#else
        int err = errno;
        if (sent < 0) {
            LOG_ERROR("PacketForwarder::send_delay_once: sendto() FAILED, errno=%d (%s), expected=%d",
                      err, strerror(err), pktLen);
        } else {
            LOG_ERROR("PacketForwarder::send_delay_once: partial send, Sent=%d, expected=%d, errno=%d (%s)",
                      sent, pktLen, err, strerror(err));
        }
#endif
    }

    // If Proto allocates byArrPkt dynamically, free it here if needed:
    // delete[] byArrPkt;
}

void PacketForwarder::SendCoOrdinateOverTcp(iface devieType)
{
    uint iaddr=0x206C;
    double now = std::chrono::duration<double>( std::chrono::system_clock::now().time_since_epoch()).count();
    double t = now - m_startTime;

    Position p = generate_position(t);

    double dist_m = 0.0;
    int    delay_us = 0;

    compute_delay(m_Xtp, m_Ytp, m_Ztp, p.x, p.y, p.z, dist_m, delay_us);

    LOG_INFO("send_delay_once: m_startTime=%.6f, now=%.6f, t=%.6f | Pos(%.3f, %.3f, %.3f) -> TP(%.3f, %.3f, %.3f) | dist=%.3f m | delay_us=%d",
             m_startTime,
             now,
             t,
             p.x, p.y, p.z,
             m_Xtp, m_Ytp, m_Ztp,
             dist_m,
             delay_us);

    Utils::RegWrite(devieType,iaddr,delay_us);
}
