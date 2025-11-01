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

bool EthernetSocketPL1G::connectSocket(const std::string& localIp, uint16_t remotePort)
{
    LOG_TO_FILE("EthernetSocketPL1G::connectSocket() <ENTER>");

    // Create socket
    sockFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockFd == INVALID_SOCKET) {
        LOG_ERROR("Failed to create TCP socket. WSAError: %d", WSAGetLastError());
        return false;
    }

    // Set non-blocking mode
    u_long mode = 1;
    if (ioctlsocket(sockFd, FIONBIO, &mode) != NO_ERROR) {
        LOG_ERROR("Failed to set non-blocking mode. WSAError: %d", WSAGetLastError());
        closeSocket();
        return false;
    }

    // Prepare remote address
    sockaddr_in remoteAddr{};
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons(remotePort);
    if (inet_pton(AF_INET, RemoteIP.toStdString().c_str(), &remoteAddr.sin_addr) <= 0) {
        LOG_ERROR("Invalid remote IP address: %s", RemoteIP.toStdString().c_str());
        closeSocket();
        return false;
    }

    // Initiate non-blocking connect
    int result = connect(sockFd, reinterpret_cast<sockaddr*>(&remoteAddr), sizeof(remoteAddr));
    if (result == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK && err != WSAEINPROGRESS) {
            LOG_ERROR("Immediate connect failed. WSAError: %d", err);
            closeSocket();
            return false;
        }

        // Wait for write readiness
        fd_set writeSet;
        FD_ZERO(&writeSet);
        FD_SET(sockFd, &writeSet);
        TIMEVAL timeout{5, 0}; // 5 seconds

        result = select(0, nullptr, &writeSet, nullptr, &timeout);
        if (result <= 0) {
            LOG_ERROR("Connection timeout or select() error. WSAError: %d", WSAGetLastError());
            closeSocket();
            return false;
        }

        // Check for socket errors
        int so_error = 0;
        int len = sizeof(so_error);
        if (getsockopt(sockFd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&so_error), &len) != 0 || so_error != 0) {
            LOG_ERROR("Socket error after connect. Code: %d", so_error);
            closeSocket();
            return false;
        }
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

bool EthernetSocketPL1G::waitForWriteReady(SOCKET fd, int timeoutSec) {
    fd_set writeSet;
    FD_ZERO(&writeSet);
    FD_SET(fd, &writeSet);
    TIMEVAL timeout{timeoutSec, 0};
    int result = select(0, nullptr, &writeSet, nullptr, &timeout);
    return result > 0;
}

bool EthernetSocketPL1G::waitForReadReady(SOCKET fd, int timeoutSec) {
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(fd, &readSet);
    TIMEVAL timeout{timeoutSec, 0};
    int result = select(0, &readSet, nullptr, nullptr, &timeout);
    return result > 0;
}
bool EthernetSocketPL1G::sendData(const char* data, int datalen)
{
    if (sockFd < 0 || data == nullptr || datalen <= 0) {
        LOG_ERROR("Invalid socket or data parameters. sockFd=%d, datalen=%d", sockFd, datalen);
        return false;
    }

    int totalSent = 0;
    const int timeoutSec = 2; // configurable per network conditions
    while (totalSent < datalen)
    {
        ssize_t sent = send(sockFd, data + totalSent, datalen - totalSent, 0);
        if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                LOG_INFO("send() would block, retrying...");
                continue;
            }
            LOG_ERROR("send() failed. Error: %d (%s)", errno, strerror(errno));
            return false;
        }
        totalSent += sent;
        //LOG_INFO("Sent %zd bytes, totalSent=%d", sent, totalSent);
    }

    return true;
}

bool EthernetSocketPL1G::receiveData(char* buffer, int bufferSize, int& receivedLen)
{
    receivedLen = 0;

    if (sockFd == INVALID_SOCKET || buffer == nullptr || bufferSize <= 0) {
        LOG_ERROR("Invalid receive parameters. sockFd=%llu, bufferSize=%d", static_cast<UINT_PTR>(sockFd), bufferSize);
        return false;
    }

    const int timeoutSec = 2;
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(sockFd, &readSet);

    TIMEVAL timeout{};
    timeout.tv_sec = timeoutSec;
    timeout.tv_usec = 0;
    int selResult = select(0, &readSet, nullptr, nullptr, &timeout);
    if (selResult <= 0) {
        LOG_ERROR("select() failed or timed out. WSAError: %d", WSAGetLastError());
        return false;
    }
    int recvLen = recv(sockFd, buffer, bufferSize, 0);
    if (recvLen == SOCKET_ERROR) {
        LOG_ERROR("recv() failed. WSAError: %d", WSAGetLastError());
        return false;
    }
    receivedLen = recvLen;
    //LOG_INFO("Received %d bytes", receivedLen);
    return true;
}

bool EthernetSocketPL1G::receivePacketWithSync2(char* buffer, int want, int &receivedLen)
{
    receivedLen = 0;
    if (!buffer || want <= 2) return false;

    char window[2];
    int got = 0, lowlvl = 0;

    // Prime read
    if (!receiveData(window, 2, lowlvl) || lowlvl < 2) return false;

    const int maxSyncAttempts = 1000; // configurable
    int attempts = 0;

    while (attempts++ < maxSyncAttempts)
    {
        if (static_cast<uint8_t>(window[0]) == 0xAAu && static_cast<uint8_t>(window[1]) == 0x88u) {
            buffer[0] = window[0];
            buffer[1] = window[1];
            got = 2;
            int remain = want - 2;

            while (remain > 0)
            {
                int chunk = 0;
                if (!receiveData(buffer + got, remain, chunk)) return false;
                if (chunk == 0)
                {
                    LOG_ERROR("receiveData returned 0 bytes mid-packet");
                    receivedLen = got;
                    return false;
                }
                got += chunk;
                remain -= chunk;
            }
            receivedLen = want;
            return true;
        }
        // Shift window
        window[0] = window[1];
        if (!receiveData(&window[1], 1, lowlvl) || lowlvl < 1) return false;
    }
    LOG_ERROR("Sync pattern not found after %d attempts", maxSyncAttempts);
    return false;
}

ssize_t EthernetSocketPL1G::receiveDataSafe(char* buffer, int bufferSize)
{
    if (sockFd < 0 || buffer == nullptr || bufferSize <= 0) {
        LOG_ERROR("receiveDataSafe: Invalid parameters sockFd:%d buffer:%p bufferSize:%d", sockFd, buffer, bufferSize);
        return -1;
    }

    while (true) {
        ssize_t n = ::recv(sockFd, buffer, static_cast<size_t>(bufferSize), 0);
        if (n > 0) {
            return n; // bytes read
        }

        if (n == 0) {
            // orderly shutdown / EOF
            LOG_INFO("receiveDataSafe: connection closed by peer (EOF)");
            return 0;
        }

        // n < 0: error
        if (errno == EINTR) {
            continue; // interrupted, retry
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // non-blocking socket would block; indicate no data now
            LOG_INFO("receiveDataSafe: would block (EAGAIN/EWOULDBLOCK)");
            return 0;
        }

        LOG_ERROR("receiveDataSafe: recv() failed errno:%d (%s)", errno, strerror(errno));
        return -1; // fatal error
    }
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
        LOG_INFO("EthernetSocketPL1G::Create() <ALLOCATING> IP:%s PORT:%d LPORT:%d RemoteIP %s", localIp.c_str(), port, localPort,remoteIP.toStdString().c_str());
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
