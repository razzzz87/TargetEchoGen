#ifndef ETHERNETSOCKET_H
#define ETHERNETSOCKET_H

#include <string>
#include <vector>
#include <cstdint>

#include <QString>
#include <QHostAddress> // optional, keeps Qt types usable

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
using sock_t = SOCKET;
constexpr sock_t INVALID_SOCK = INVALID_SOCKET;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
using sock_t = int;
constexpr sock_t INVALID_SOCK = -1;
#endif

class EthernetSocket
{
public:
    // Factory / lifecycle
    static EthernetSocket* getInstance();
    static EthernetSocket* Create(const std::string& localIp,
                                  uint16_t localPort,
                                  const QString& RemoteIP,
                                  quint16 Port);
    static void destroyInstance();

    // Socket operations
    bool bindSocket(const std::string& localIp, uint16_t localPort);
    bool sendData(const std::vector<uint8_t>& data, const std::string& destIp, uint16_t destPort);
    bool sendData(const char* data, int datalen, const std::string& destIp, uint16_t destPort);
    bool receiveData(std::vector<uint8_t>& buffer, std::string& senderIp, uint16_t& senderPort);
    bool receiveData(char* buffer, int len, std::string& senderIp, uint16_t& senderPort);
    bool closeSocket();
    bool getConnStatus() const;

    // metadata
    std::string getInterfaceLabel() const;
    void setInterfaceLabel(const std::string& label);

    // Qt-friendly members retained for existing code
    QString RemoteIP;
    quint16 Port;
    QString LocalIP;

    ~EthernetSocket();

private:
    EthernetSocket();                     // private ctor
    bool initPlatformSockets();           // Windows: WSAStartup; Linux: no-op
    void cleanupPlatformSockets();        // Windows: WSACleanup; Linux: no-op
    bool makeSocketNonBlocking(bool nonBlocking);

    // platform-neutral socket handle and helpers
    sock_t sockFd;
    bool IsConnected;
    std::string interfaceLabel;

    // local address storage (for bind)
    sockaddr_in localAddr;

    // singleton
    static EthernetSocket *instance;
};

#endif // ETHERNETSOCKET_H
