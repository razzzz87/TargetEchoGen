#pragma once

#include <QThread>
#include <QString>
#include <atomic>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <cmath>

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
    char     reserved[76];   // 76 bytes like Python '76s'
};
#pragma pack(pop)

static_assert(sizeof(UdpPayload) == 108, "UdpPayload must be 108 bytes");

class PacketForwarder : public QThread
{
    Q_OBJECT

public:
    // srcIp/srcPort: IMS server you talk to (client mode, send READY, then recv)
    // dstIp/dstPort: destination server where you forward packets (client mode)
    // csvPath: log file for received packets
    explicit PacketForwarder(const QString &srcIp,
                             quint16 srcPort,
                             const QString &dstIp,
                             quint16 dstPort,
                             const QString &csvPath,
                             QObject *parent = nullptr);

    ~PacketForwarder() override;

    void stop();

    // Target coordinates (like Xtp/Ytp/Ztp in Python client)
    void setTarget(double Xtp, double Ytp, double Ztp);

protected:
    void run() override;

private:
    // Upstream IMS server (where we receive from)
    QString m_srcIp;
    quint16 m_srcPort;

    // Downstream forward server (where we send to)
    QString m_dstIp;
    quint16 m_dstPort;

    QString m_csvPath;

    std::atomic<bool> m_stopRequested;

    int m_srcSock;   // socket A: send READY + recv packets
    int m_dstSock;   // socket B: forward packets

#ifdef _WIN32
    bool m_wsaInitialized;
#endif

    // Target position used to compute distance & delay
    double m_Xtp = 10.0;
    double m_Ytp = 10.0;
    double m_Ztp = 10.0;

    std::ofstream m_csv;

    // Helpers
    bool initialize();
    void relayLoop();
    void cleanup();

    bool openCsv(const std::string &path);
    void writeCsvRow(const std::string &ts,
                     uint32_t msg_num,
                     uint32_t id_field,
                     double x, double y, double z,
                     double dist, double delay_s);

    std::string now_utc_iso8601();
    void compute_delay(double Xtp, double Ytp, double Ztp,
                       double x, double y, double z,
                       double &dist, double &delay);
};
