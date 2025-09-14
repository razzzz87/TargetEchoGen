#ifndef ETHERNETSOCKET_H
#define ETHERNETSOCKET_H
#include <string>
#include <vector>
#include <cstdint>
#include <QString>

class EthernetSocket
{
public:
    static EthernetSocket* getInstance();
    static EthernetSocket* Create(const std::string& localIp, uint16_t localPort, QString RemotIP, quint16 Port);
    static void destroyInstance();
    bool bindSocket(const std::string& localIp, uint16_t localPort);
    bool sendData(const std::vector<uint8_t>& data, const std::string& destIp, uint16_t destPort);
    bool sendData(const char* data, int datalen, const std::string& destIp, uint16_t destPort);
    bool receiveData(std::vector<uint8_t>& buffer, std::string& senderIp, uint16_t& senderPort);
    bool receiveData(char *buffer, int len, std::string& senderIp, uint16_t& senderPort);
    bool closeSocket();
    bool getConnStatus();

    std::string getInterfaceLabel() const;
    void setInterfaceLabel(const std::string& label);
    QString RemoteIP;
    quint16 Port;
    QString LocalIP;
    ~EthernetSocket();
private:
    EthernetSocket();
    int sockFd;
    bool IsConnected;
    std::string interfaceLabel;
    static EthernetSocket *instance;
};

#endif // ETHERNETSOCKET_H
