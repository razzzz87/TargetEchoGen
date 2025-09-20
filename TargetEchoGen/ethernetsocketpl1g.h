#ifndef ETHERNETSOCKETPL1G_H
#define ETHERNETSOCKETPL1G_H
#include <QString>

class EthernetSocketPL1G {
public:
    // Singleton access
    static EthernetSocketPL1G* getInstance();
    static EthernetSocketPL1G* Create(const std::string& localIp, uint16_t localPort, QString remoteIP, quint16 port);
    static void destroyInstance();

    // TCP-specific methods
    bool connectSocket(const std::string& localIp, uint16_t remotePort);
    bool sendData(const char* data, int datalen); // TCP doesn't need dest IP/port
    bool receiveData(char* buffer, int bufferSize, int& receivedLen);

    // Optional utilities
    bool setSocketBufferSize(int recvSize, int sendSize);
    bool closeSocket();
    bool getConnStatus() const;

    // Interface metadata
    std::string getInterfaceLabel() const;
    void setInterfaceLabel(const std::string& label);

    // Public connection info
    QString RemoteIP;
    quint16 Port;
    QString LocalIP;

    ~EthernetSocketPL1G();

private:
    EthernetSocketPL1G(); // Private constructor for singleton
    int sockFd;
    bool IsConnected;
    std::string interfaceLabel;
    static EthernetSocketPL1G* instance;
};

#endif // ETHERNETSOCKETPL1G_H
