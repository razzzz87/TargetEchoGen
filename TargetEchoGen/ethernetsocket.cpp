#include "ethernetsocket.h"
#include "log.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <cstring>
#endif

#include <string>
#include <vector>

#ifdef _WIN32
using sock_err_t = int;
#else
using sock_err_t = int;
#endif

EthernetSocket* EthernetSocket::instance = nullptr;

EthernetSocket::EthernetSocket()
    : sockFd(INVALID_SOCK), IsConnected(false)
{
#ifdef _WIN32
    WSADATA wsaData;
    int rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc != 0) {
        LOG_ERROR("[EthernetSocket] WSAStartup failed: %d", rc);
    } else {
        LOG_INFO("[EthernetSocket] WSAStartup succeeded");
    }
#else
    LOG_INFO("[EthernetSocket] Constructor (Linux)");
#endif
}

EthernetSocket::~EthernetSocket()
{
    LOG_INFO("[EthernetSocket] Destructor");
    closeSocket();
#ifdef _WIN32
    WSACleanup();
    LOG_INFO("[EthernetSocket] WSACleanup done");
#endif
}

bool EthernetSocket::bindSocket(const std::string& localIp, uint16_t localPort)
{
    LOG_INFO("[bindSocket] Enter | IP: %s Port: %u", localIp.c_str(), localPort);

// create UDP socket
#ifdef _WIN32
    sockFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockFd == INVALID_SOCKET) {
        LOG_ERROR("[bindSocket] socket() failed: %d", WSAGetLastError());
        return false;
    }
#else
    sockFd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (sockFd < 0) {
        LOG_ERROR("[bindSocket] socket() failed: %s", strerror(errno));
        return false;
    }
#endif

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(localPort);

    if (localIp.empty()) {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        int pton_rc = inet_pton(AF_INET, localIp.c_str(), &addr.sin_addr);
        if (pton_rc != 1) {
#ifdef _WIN32
            LOG_ERROR("[bindSocket] inet_pton failed for %s | Error: %d", localIp.c_str(), WSAGetLastError());
            closesocket(sockFd);
            sockFd = INVALID_SOCKET;
#else
            LOG_ERROR("[bindSocket] inet_pton failed for %s | Error: %s", localIp.c_str(), strerror(errno));
            ::close(sockFd);
            sockFd = -1;
#endif
            return false;
        }
    }

    if (bind(sockFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
#ifdef _WIN32
        int err = WSAGetLastError();
        LOG_ERROR("[bindSocket] bind() failed | Error: %d", err);
        closesocket(sockFd);
        sockFd = INVALID_SOCKET;
#else
        int err = errno;
        LOG_ERROR("[bindSocket] bind() failed | Error: %d (%s)", err, strerror(err));
        ::close(sockFd);
        sockFd = -1;
#endif
        return false;
    }

    localAddr = addr;
    IsConnected = true;
    LOG_INFO("[bindSocket] Bound successfully to %s:%u", localIp.empty() ? "*" : localIp.c_str(), localPort);
    return true;
}

bool EthernetSocket::sendData(const std::vector<uint8_t>& data, const std::string& destIp, uint16_t destPort)
{
    LOG_INFO("[sendData(vector)] Enter | Len: %zu Dest: %s:%u", data.size(), destIp.c_str(), destPort);
    if (sockFd == INVALID_SOCK) {
        LOG_ERROR("[sendData] invalid socket");
        return false;
    }

    sockaddr_in dest{};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(destPort);

    if (inet_pton(AF_INET, destIp.c_str(), &dest.sin_addr) != 1) {
#ifdef _WIN32
        LOG_ERROR("[sendData] inet_pton failed for %s | Error: %d", destIp.c_str(), WSAGetLastError());
#else
        LOG_ERROR("[sendData] inet_pton failed for %s | Error: %s", destIp.c_str(), strerror(errno));
#endif
        return false;
    }

#ifdef _WIN32
    int sent = sendto(sockFd,
                      reinterpret_cast<const char*>(data.data()),
                      static_cast<int>(data.size()),
                      0,
                      reinterpret_cast<sockaddr*>(&dest),
                      static_cast<int>(sizeof(dest)));
    if (sent == SOCKET_ERROR) {
        LOG_ERROR("[sendData] sendto failed | Error: %d", WSAGetLastError());
        return false;
    }
#else
    ssize_t sent = sendto(sockFd,
                          reinterpret_cast<const char*>(data.data()),
                          data.size(),
                          0,
                          reinterpret_cast<sockaddr*>(&dest),
                          sizeof(dest));
    if (sent < 0) {
        LOG_ERROR("[sendData] sendto failed | Error: %s", strerror(errno));
        return false;
    }
#endif

    LOG_INFO("[sendData] Sent %d bytes", static_cast<int>(sent));
    return static_cast<size_t>(sent) == data.size();
}

bool EthernetSocket::sendData(const char* data, int datalen, const std::string& destIp, uint16_t destPort)
{
    LOG_INFO("[sendData(char*)] Enter | Len: %d Dest: %s:%u", datalen, destIp.c_str(), destPort);
    if (sockFd == INVALID_SOCK || data == nullptr || datalen <= 0) {
        LOG_ERROR("[sendData(char*)] invalid params sockFd=%d datalen=%d", static_cast<int>(sockFd), datalen);
        return false;
    }

    sockaddr_in dest{};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(destPort);
    if (inet_pton(AF_INET, destIp.c_str(), &dest.sin_addr) != 1) {
#ifdef _WIN32
        LOG_ERROR("[sendData(char*)] inet_pton failed for %s | Error: %d", destIp.c_str(), WSAGetLastError());
#else
        LOG_ERROR("[sendData(char*)] inet_pton failed for %s | Error: %s", destIp.c_str(), strerror(errno));
#endif
        return false;
    }

#ifdef _WIN32
    int sent = sendto(sockFd, data, datalen, 0, reinterpret_cast<sockaddr*>(&dest), static_cast<int>(sizeof(dest)));
    if (sent == SOCKET_ERROR) {
        LOG_ERROR("[sendData(char*)] sendto failed | Error: %d", WSAGetLastError());
        return false;
    }
#else
    ssize_t sent = sendto(sockFd, data, static_cast<size_t>(datalen), 0, reinterpret_cast<sockaddr*>(&dest), sizeof(dest));
    if (sent < 0) {
        LOG_ERROR("[sendData(char*)] sendto failed | Error: %s", strerror(errno));
        return false;
    }
#endif

    LOG_INFO("[sendData(char*)] Sent %d bytes", static_cast<int>(sent));
    return static_cast<int>(sent) == datalen;
}

bool EthernetSocket::receiveData(std::vector<uint8_t>& buffer, std::string& senderIp, uint16_t& senderPort)
{
    LOG_INFO("[receiveData(vector)] Enter");
    if (sockFd == INVALID_SOCK) {
        LOG_ERROR("[receiveData] invalid socket");
        return false;
    }

    buffer.resize(2048);
    sockaddr_in sender{};
    socklen_t addrLen = sizeof(sender);

#ifdef _WIN32
    int recvd = recvfrom(sockFd, reinterpret_cast<char*>(buffer.data()), static_cast<int>(buffer.size()), 0,
                         reinterpret_cast<sockaddr*>(&sender), &addrLen);
    if (recvd == SOCKET_ERROR || recvd == 0) {
        LOG_ERROR("[receiveData] recvfrom failed | Error: %d", WSAGetLastError());
        return false;
    }
    buffer.resize(static_cast<size_t>(recvd));
#else
    ssize_t recvd = recvfrom(sockFd, reinterpret_cast<char*>(buffer.data()), buffer.size(), 0,
                             reinterpret_cast<sockaddr*>(&sender), &addrLen);
    if (recvd < 0) {
        LOG_ERROR("[receiveData] recvfrom failed | Error: %s", strerror(errno));
        return false;
    }
    if (recvd == 0) {
        LOG_ERROR("[receiveData] recvfrom returned 0");
        return false;
    }
    buffer.resize(static_cast<size_t>(recvd));
#endif

    char ipStr[INET_ADDRSTRLEN] = {0};
    if (inet_ntop(AF_INET, &sender.sin_addr, ipStr, sizeof(ipStr)) == nullptr) {
#ifdef _WIN32
        LOG_ERROR("[receiveData] inet_ntop failed | Error: %d", WSAGetLastError());
#else
        LOG_ERROR("[receiveData] inet_ntop failed | Error: %s", strerror(errno));
#endif
        return false;
    }
    senderIp = ipStr;
    senderPort = ntohs(sender.sin_port);

    LOG_INFO("[receiveData(vector)] Received %zu bytes from %s:%u", buffer.size(), senderIp.c_str(), senderPort);
    return true;
}

bool EthernetSocket::receiveData(char* buffer, int len, std::string& senderIp, uint16_t& senderPort)
{
    LOG_INFO("[receiveData(char*)] Enter | MaxLen: %d", len);
    if (sockFd == INVALID_SOCK || buffer == nullptr || len <= 0) {
        LOG_ERROR("[receiveData(char*)] invalid params");
        return false;
    }

    sockaddr_in sender{};
    socklen_t addrLen = sizeof(sender);

#ifdef _WIN32
    int recvd = recvfrom(sockFd, buffer, len, 0, reinterpret_cast<sockaddr*>(&sender), &addrLen);
    if (recvd == SOCKET_ERROR || recvd == 0) {
        LOG_ERROR("[receiveData(char*)] recvfrom failed | Error: %d", WSAGetLastError());
        return false;
    }
#else
    ssize_t recvd = recvfrom(sockFd, buffer, static_cast<size_t>(len), 0, reinterpret_cast<sockaddr*>(&sender), &addrLen);
    if (recvd < 0) {
        LOG_ERROR("[receiveData(char*)] recvfrom failed | Error: %s", strerror(errno));
        return false;
    }
    if (recvd == 0) {
        LOG_ERROR("[receiveData(char*)] recvfrom returned 0");
        return false;
    }
#endif

    char ipStr[INET_ADDRSTRLEN] = {0};
    if (inet_ntop(AF_INET, &sender.sin_addr, ipStr, sizeof(ipStr)) == nullptr) {
#ifdef _WIN32
        LOG_ERROR("[receiveData(char*)] inet_ntop failed | Error: %d", WSAGetLastError());
#else
        LOG_ERROR("[receiveData(char*)] inet_ntop failed | Error: %s", strerror(errno));
#endif
        return false;
    }

    senderIp = ipStr;
    senderPort = ntohs(sender.sin_port);

    LOG_INFO("[receiveData(char*)] Received %d bytes from %s:%u", static_cast<int>(recvd), senderIp.c_str(), senderPort);
    return true;
}

bool EthernetSocket::getConnStatus() const
{
    return IsConnected;
}

bool EthernetSocket::closeSocket()
{
    LOG_INFO("[closeSocket] Enter");
    if (sockFd != INVALID_SOCK) {
#ifdef _WIN32
        closesocket(sockFd);
        sockFd = INVALID_SOCKET;
#else
        ::close(sockFd);
        sockFd = -1;
#endif
        IsConnected = false;
        LOG_INFO("[closeSocket] Socket closed");
        return true;
    }
    LOG_INFO("[closeSocket] Socket already closed");
    return true;
}

std::string EthernetSocket::getInterfaceLabel() const {
    return interfaceLabel;
}

void EthernetSocket::setInterfaceLabel(const std::string& label) {
    interfaceLabel = label;
}

EthernetSocket* EthernetSocket::getInstance()
{
    if (instance) {
        LOG_INFO("[getInstance] returning existing instance");
    } else {
        LOG_INFO("[getInstance] instance is null");
    }
    return instance;
}

EthernetSocket* EthernetSocket::Create(const std::string& localIp, uint16_t localPort, const QString& RemoteIP, quint16 Port)
{
    LOG_INFO("[Create] Enter | localIp=%s localPort=%u RemoteIP=%s RemotePort=%u",
             localIp.c_str(), localPort, RemoteIP.toStdString().c_str(), static_cast<unsigned>(Port));

    if (instance == nullptr)
    {
        LOG_INFO("[Create] Allocating instance");
        instance = new EthernetSocket();
        instance->setInterfaceLabel("ETH1G");
        instance->RemoteIP = RemoteIP;
        instance->Port = Port;
        instance->LocalIP = QString::fromStdString(localIp);

        // FIX: pass localPort to bind, not remote Port
        if (!instance->bindSocket(localIp, localPort))
        {
            LOG_ERROR("[Create] bindSocket failed for %s:%u", localIp.c_str(), localPort);
            delete instance;
            instance = nullptr;
            return nullptr;
        }
        LOG_INFO("[Create] Instance created and bound to %s:%u", localIp.c_str(), localPort);
    } else {
        LOG_INFO("[Create] Returning existing instance");
    }

    LOG_INFO("[Create] Exit");
    return instance;
}

void EthernetSocket::destroyInstance()
{
    LOG_INFO("[destroyInstance] Enter");
    if (instance) {
        instance->closeSocket();
        delete instance;
        instance = nullptr;
        LOG_INFO("[destroyInstance] Instance destroyed");
    } else {
        LOG_INFO("[destroyInstance] No instance to destroy");
    }
}
