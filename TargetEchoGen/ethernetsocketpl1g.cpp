#include "ethernetsocketpl1g.h"
#include "log.h"
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

#include <iostream>
#include <cstring>

EthernetSocketPL1G *EthernetSocketPL1G::instance = nullptr;

EthernetSocketPL1G::EthernetSocketPL1G() : sockFd(-1) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    IsConnected = false;
}

EthernetSocketPL1G::~EthernetSocketPL1G() {
    closeSocket();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool EthernetSocketPL1G::connectSocket(const std::string& localIp, uint16_t remotePort) {
    LOG_TO_FILE("EthernetSocketPL1G::connectSocket() <ENTER>");
    sockFd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    if (sockFd < 0) {
        LOG_ERROR("Failed to create TCP socket");
        return false;
    }

    sockaddr_in remoteAddr{};
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons(remotePort);
    if (inet_pton(AF_INET, RemoteIP.toStdString().c_str(), &remoteAddr.sin_addr) <= 0) {
        LOG_ERROR("Invalid remote IP address: %s", RemoteIP.toStdString().c_str());
        closeSocket();
        return false;
    }

    if (connect(sockFd, reinterpret_cast<sockaddr*>(&remoteAddr), sizeof(remoteAddr)) < 0) {
        LOG_ERROR("Failed to connect to %s:%d", RemoteIP.toStdString().c_str(), remotePort);
        closeSocket();
        return false;
    }

    IsConnected = true;
    LOG_INFO("EthernetSocketPL1G::connectSocket() <CONNECTED>");
    return true;
}

bool EthernetSocketPL1G::setSocketBufferSize(int recvSize, int sendSize)
{
    if (sockFd <= 0) {
        LOG_ERROR("Socket buffer config failed: invalid socket descriptor.");
        return false;
    }

    if (setsockopt(sockFd, SOL_SOCKET, SO_RCVBUF, (const char*)&recvSize, sizeof(recvSize)) < 0) {
        LOG_ERROR("Failed to set receive buffer size. Error: %d (%s)", errno, strerror(errno));
        return false;
    }

    if (setsockopt(sockFd, SOL_SOCKET, SO_SNDBUF, (const char*)&sendSize, sizeof(sendSize)) < 0) {
        LOG_ERROR("Failed to set send buffer size. Error: %d (%s)", errno, strerror(errno));
        return false;
    }

    LOG_INFO("Socket buffer sizes applied: RX=%d bytes, TX=%d bytes", recvSize, sendSize);
    return true;
}

bool EthernetSocketPL1G::sendData(const char* data, int datalen) {
    if (sockFd < 0 || data == nullptr || datalen <= 0) {
        LOG_ERROR("Invalid socket or data parameters. sockFd=%d, datalen=%d", sockFd, datalen);
        return false;
    }

    ssize_t sent = send(sockFd, data, datalen, 0);
    if (sent < 0) {
        LOG_ERROR("send() failed. Error: %d (%s)", errno, strerror(errno));
        return false;
    }

    return sent == datalen;
}

bool EthernetSocketPL1G::receiveData(char* buffer, int bufferSize, int& receivedLen) {
    if (sockFd < 0 || buffer == nullptr || bufferSize <= 0) {
        LOG_ERROR("Invalid receive parameters.");
        return false;
    }

    receivedLen = recv(sockFd, buffer, bufferSize, 0);
    if (receivedLen < 0) {
        LOG_ERROR("recv() failed. Error: %d (%s)", errno, strerror(errno));
        return false;
    }
    return true;
}

bool EthernetSocketPL1G::getConnStatus() const {
    return IsConnected;
}

bool EthernetSocketPL1G::closeSocket() {
    if (sockFd >= 0) {
#ifdef _WIN32
        closesocket(sockFd);
#else
        close(sockFd);
#endif
        sockFd = -1;
    }

    IsConnected = false;
    LOG_INFO("EthernetSocketPL1G::closeSocket() <SOCKET CLOSED>");
    return true;
}

std::string EthernetSocketPL1G::getInterfaceLabel() const {
    return interfaceLabel;
}

void EthernetSocketPL1G::setInterfaceLabel(const std::string& label) {
    interfaceLabel = label;
}


EthernetSocketPL1G* EthernetSocketPL1G::getInstance() {
    if (!instance) {
        LOG_TO_FILE("EthernetSocketPL1G::getInstance() <INSTANCE IS NULL>");
    }
    return instance;
}

EthernetSocketPL1G* EthernetSocketPL1G::Create(const std::string& localIp, uint16_t localPort, QString remoteIP, quint16 port) {
    LOG_INFO("EthernetSocketPL1G::Create() <ENTER>");
    if (instance == nullptr) {
        LOG_INFO("EthernetSocketPL1G::Create() <ALLOCATING> IP:%s PORT:%d LPORT:%d", localIp.c_str(), port, localPort);
        instance = new EthernetSocketPL1G();
        instance->setInterfaceLabel("ETHPL1G");
        instance->RemoteIP = remoteIP;
        instance->Port = port;
        if (!instance->connectSocket(localIp, port)) {
            delete instance;
            instance = nullptr;
            LOG_INFO("EthernetSocketPL1G::Create() <FAILED TO CONNECT>");
            return nullptr;
        }
        LOG_INFO("EthernetSocketPL1G::Create() <CONNECTED SUCCESSFULLY>");
    }
    LOG_INFO("EthernetSocketPL1G::Create() <EXIT>");
    return instance;
}

void EthernetSocketPL1G::destroyInstance()
{
    instance->closeSocket();
    delete instance;
    instance = nullptr;
}
