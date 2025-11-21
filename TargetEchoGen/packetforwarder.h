#pragma once

#include <QThread>
#include <QString>
#include <atomic>
#include <cmath>
#include <random>
#include <fstream>
static constexpr double SPEED_OF_LIGHT = 299792458.0;
struct Position {
    double x;
    double y;
    double z;
};
#pragma pack(push, 1)
struct UdpPayload
{
    uint32_t id_field;
    uint32_t msg_num;
    double   x;
    double   y;
    double   z;
    uint32_t reserved;
};
#pragma pack(pop)
// Forward declarations for libpcap types (so header doesn't need pcap.h)
struct pcap;
struct pcap_dumper;
typedef struct pcap pcap_t;
typedef struct pcap_dumper pcap_dumper_t;

class PacketForwarder : public QThread
{
    Q_OBJECT

public:
    explicit PacketForwarder(const QString &interfaceName,
                             quint16 listenUdpPort,
                             const QString &forwardIp,
                             quint16 forwardUdpPort,
                             const QString &dumpFilePath,
                             QObject *parent = nullptr);

    ~PacketForwarder() override;

    // Request the thread to stop (captureLoop will exit)
    void stop();
    bool applyFilter(const QString &filterString);
    // Generate synthetic position (equivalent to your Python function)
    Position generate_position(double t);
    // Compute distance + delay
    void compute_delay(double Xtp, double Ytp, double Ztp,double x, double y, double z,double &dist, double &delay);
    double random_uniform(double a, double b);
    std::string now_utc_iso8601();
    bool openLogFile(const std::string &path);
    void LogPacket(const std::string &ts, uint32_t msg_num, uint32_t id_field, double x, double y, double z,double dist, double delay_us);
    void LogCSV(const std::string &ts, uint32_t msg_num, uint32_t id_field, double x, double y, double z, double Xtp, double Ytp, double Ztp, double dist, double delay);

protected:
    void run() override;

private:
    // Configuration
    QString        m_interfaceName;   // pcap device name
    quint16        m_listenUdpPort;   // UDP destination port to capture
    QString        m_forwardIp;       // IP to forward UDP payloads to
    quint16        m_forwardUdpPort;  // UDP destination port to forward to
    QString        m_dumpFilePath;    // .pcap dump file

    // Runtime state
    std::atomic<bool> m_stopRequested;
    pcap_t           *m_pcapHandle;
    pcap_dumper_t    *m_pcapDumper;
    int               m_udpSock;
    std::ofstream g_logFile;

#ifdef _WIN32
    bool              m_wsaInitialized;
#endif

    // Helpers
    bool initialize();      // one-time setup
    void captureLoop();     // continuous pcap read + forward
    void cleanup();         // release all resources

    double Xtp = 0.0;
    double Ytp = 0.0;
    double Ztp = 0.0;
};
