#pragma once

#include <QThread>
#include <QString>
#include <atomic>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <cstdint>
#include "Utils.h"
#include "connectionctx.h"
#include <QtGlobal>

static const int ETH_DATA_SIZE = 1456;    // bytes of payload per UDP packet
static const int PROTO_OVERHEAD = 12;     // mPktBulkWrite header size

// 1) Struct to carry all measurement info
struct RelayMeasurement
{
    double Xtp;
    double Ytp;
    double Ztp;

    double x;
    double y;
    double z;

    double dist_m;
    int    delay_us;

    quint32 msg_num;
    quint32 id_field;

    double startTime;   // optional: timestamp as double
};
// 2) Make it known to Qt meta-object system
Q_DECLARE_METATYPE(RelayMeasurement)

struct Position {
    double x;
    double y;
    double z;
};

static constexpr double SPEED_OF_LIGHT = 299792458.0;

// IMS payload: matches Python FORMAT = '!I I d d d 76s'
#pragma pack(push, 1)
struct UdpPayload
{
    uint32_t id_field;
    uint32_t msg_num;
    double   x;
    double   y;
    double   z;
    char     reserved[76];
};
#pragma pack(pop)

static_assert(sizeof(UdpPayload) == 108, "UdpPayload must be 108 bytes");

class PacketForwarder : public QThread
{
    Q_OBJECT

public:
    // Constructor:
    //   UDP receive (IMS/source): srcIp/srcPort   -> m_udpRecvSock (bind here)
    //   UDP transmit:             txIp/txPort     -> m_udpTxSock (delay + file)
    //   csvPath:                  CSV log path
    explicit PacketForwarder(const QString &srcIp,
                             quint16 srcPort,
                             const QString &txIp,
                             quint16 txPort,
                             const QString &csvPath,
                             QObject *parent = nullptr);

    ~PacketForwarder() override;

    void stop();
    void SendCoOrdinateOverTcp(iface devieType);
    // Target coordinates used for distance & delay computation
    void setTarget(double Xtp, double Ytp, double Ztp);
    void setTargetCoordinates(double x, double y, double z);

    // CSV rotation helper
    void rotateCsvIfNeeded();

    // ---- Delay + synthetic motion ----
    double   rand_uniform();
    Position generate_position(double t);
    double readDoubleBE(const void* ptr);
    uint32_t readU32BE(const void* ptr);

    // Distance & delay (µs)
    void compute_delay(double Xtp, double Ytp, double Ztp,
                       double x, double y, double z,
                       double &dist_m, int &delay_us);

    // One-shot delay send (UDP):
    // Uses synthetic position = generate_position(time_since_initialize)
    void send_delay_once(double Xtp, double Ytp, double Ztp);

    // Send coordinate/file data over UDP using the same TX socket
    void sendCoordinateFileUdp(const QString &filePath);
    iface getSelectedDeviceType();
    bool sendCoordinateChunk(QByteArray &chunk, uint32_t &startAddress, const sockaddr_in &txAddr);
    // Open sockets and prepare state
    bool initialize();
    double m_startTime;

protected:
    void run() override;

signals:
    void measurementUpdated(const RelayMeasurement &m);

private:
    // UDP receive (IMS/source)
    QString m_srcIp;
    quint16 m_srcPort;

    // UDP transmit (delay + file)
    QString m_txIp;
    quint16 m_txPort;

    // CSV log path
    QString m_csvPath;

    std::atomic<bool> m_stopRequested;
    bool m_stop;

    // Sockets
    int m_udpRecvSock;   // UDP receive (position packets)
    int m_udpTxSock;     // UDP transmit (delay packets + file data)

#ifdef _WIN32
    bool m_wsaInitialized;
#endif

    // Target coordinates
    double m_Xtp = 10.0;
    double m_Ytp = 10.0;
    double m_Ztp = 10.0;

    // Start time used by generate_position(t) via send_delay_once()
    //double m_startTime = 0.0;

    std::ofstream m_csv;

    // Internal helpers
    void relayLoop();
    void cleanup();

    bool openCsv(const std::string &path);
    void writeCsvRow(const std::string &ts,
                     uint32_t msg_num,
                     uint32_t id_field,
                     double x, double y, double z,
                     double dist, double delay_s);

    std::string now_utc_iso8601();
};
