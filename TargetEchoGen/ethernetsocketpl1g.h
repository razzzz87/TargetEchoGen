#ifndef ETHERNETSOCKETPL1G_H
#define ETHERNETSOCKETPL1G_H
#include <QString>

class EthernetSocketPL1G
{
public:
    static EthernetSocketPL1G* getInstance();
    static EthernetSocketPL1G* Create(const std::string& localIp, uint16_t localPort, QString RemotIP, quint16 Port);
    static void destroyInstance();
    bool bindSocket(const std::string& localIp, uint16_t localPort);
    bool sendData(const std::vector<uint8_t>& data, const std::string& destIp, uint16_t destPort);
    bool setSocketBufferSize(int recvSize, int sendSize);
    bool sendData(const char* data, int datalen, const std::string& destIp, uint16_t destPort);
    bool receiveData(std::vector<uint8_t>& buffer, std::string& senderIp, uint16_t& senderPort);
    int receiveData(char *buffer, int len, std::string& senderIp, uint16_t& senderPort);
    bool closeSocket();
    bool getConnStatus();

    std::string getInterfaceLabel() const;
    void setInterfaceLabel(const std::string& label);
    QString RemoteIP;
    quint16 Port;
    QString LocalIP;
    ~EthernetSocketPL1G();
private:
    EthernetSocketPL1G();
    int sockFd;
    bool IsConnected;
    std::string interfaceLabel;
    static EthernetSocketPL1G *instance;
};

#endif // ETHERNETSOCKETPL1G_H
