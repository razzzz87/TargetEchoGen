#ifndef ETHERNETSOCKET10G_H
#define ETHERNETSOCKET10G_H
#include <string>
#include <vector>
#include <cstdint>
#include <QString>

class EthernetSocket10G
{
public:
    static EthernetSocket10G* getInstance();
    static EthernetSocket10G* Create(const std::string& localIp, uint16_t localPort, QString RemotIP, quint16 Port);
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
    ~EthernetSocket10G();
private:
    EthernetSocket10G();
    int sockFd;
    bool IsConnected;
    std::string interfaceLabel;
    static EthernetSocket10G *instance;
};

#endif // ETHERNETSOCKET10G_H
