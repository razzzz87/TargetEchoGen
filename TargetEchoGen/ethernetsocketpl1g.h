#ifndef ETHERNETSOCKETPL1G_H
#define ETHERNETSOCKETPL1G_H
#include <QString>
// Always include this first
#include <winsock2.h>
#include <ws2tcpip.h>  // for inet_pton, etc.
#include <windows.h>   // only if needed, and always after winsock2.h

#pragma comment(lib, "ws2_32.lib")  // Link with Winsock library

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
    ssize_t receiveDataSafe(char* buffer, int bufferSize);
    bool receivePacketWithSync2(char* buffer, int want, int &receivedLen);
    bool waitForReadReady(SOCKET fd, int timeoutSec);
    bool waitForWriteReady(SOCKET fd, int timeoutSec);

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
