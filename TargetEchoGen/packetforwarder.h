#pragma once

#include <QThread>
#include <QString>
#include <atomic>

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

#ifdef _WIN32
    bool              m_wsaInitialized;
#endif

    // Helpers
    bool initialize();      // one-time setup
    void captureLoop();     // continuous pcap read + forward
    void cleanup();         // release all resources
};
