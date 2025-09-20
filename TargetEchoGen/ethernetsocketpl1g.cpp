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

bool EthernetSocketPL1G::bindSocket(const std::string& localIp, uint16_t localPort) {

    LOG_TO_FILE("EthernetSocketPL1G::bindSocket() <ENTER>");
    sockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockFd < 0) {
        std::cerr << "Failed to create socket\n";
        return false;
    }

    sockaddr_in localAddr{};
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(localPort);
    //localAddr.sin_addr.s_addr = inet_addr(localIp.c_str());
    localAddr.sin_addr.s_addr = INADDR_ANY ;//inet_addr("0.0.0.0");
    LOG_TO_FILE("IP:%s PORT:%d",localIp.c_str(),localPort);
    if (bind(sockFd, reinterpret_cast<sockaddr*>(&localAddr), sizeof(localAddr)) < 0) {

        LOG_TO_FILE("Failed to bind socket to IP:%s PORT:%d",localIp.c_str(),localPort);
#ifdef _WIN32
        int errorCode = WSAGetLastError();
        LOG_TO_FILE("Failed to bind socket to IP:%s PORT:%d | Error Code: %d", localIp.c_str(), localPort, errorCode);
#else
        int errorCode = errno;
        LOG_TO_FILE("Failed to bind socket to IP:%s PORT:%d | Error Code: %d (%s)", localIp.c_str(), localPort, errorCode, strerror(errorCode));
#endif
        closeSocket();
        return false;
    }
    LOG_INFO("EthernetSocketPL1G::bindSocket() <EXIT>");
    IsConnected = true;
    return IsConnected;
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

bool EthernetSocketPL1G::sendData(const char* data, int datalen, const std::string& destIp, uint16_t destPort)
{
    //LOG_DEBUG("EthernetSocketPL1G::sendData() <ENTER>");
    if (sockFd < 0 || data == nullptr || datalen <= 0) {
        LOG_ERROR("Invalid socket or data parameters. sockFd=%d, datalen=%d", sockFd, datalen);
        return false;
    }

    sockaddr_in destAddr{};
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(destPort);

    //LOG_DEBUG("Preparing to send to IP: %s, Port: %d, DataLen: %d", destIp.c_str(), destPort, datalen);

    if (inet_pton(AF_INET, destIp.c_str(), &destAddr.sin_addr) <= 0) {
#ifdef _WIN32
        int errorCode = WSAGetLastError();
        LOG_ERROR("Invalid destination IP address: %s | Error Code: %d", destIp.c_str(), errorCode);
#else
        int errorCode = errno;
        LOG_TO_FILE("Invalid destination IP address: %s | Error Code: %d (%s)", destIp.c_str(), errorCode, strerror(errorCode));
#endif
        return false;
    }

    ssize_t sent = sendto(sockFd, data, datalen, 0, reinterpret_cast<sockaddr*>(&destAddr), sizeof(destAddr));

    if (sent < 0) {
#ifdef _WIN32
        int errorCode = WSAGetLastError();
        LOG_ERROR("sendto() failed | Error Code: %d", errorCode);
#else
        int errorCode = errno;
        LOG_TO_FILE("sendto() failed | Error Code: %d (%s)", errorCode, strerror(errorCode));
#endif
        return false;
    }

    //LOG_DEBUG("Sent %zd bytes to %s:%d", sent, destIp.c_str(), destPort);
    //LOG_DEBUG("EthernetSocketPL1G::sendData() <EXIT>");
    return sent == datalen;
}

bool EthernetSocketPL1G::receiveData(std::vector<uint8_t>& buffer,
                                    std::string& senderIp, uint16_t& senderPort) {
    LOG_TO_FILE("EthernetSocketPL1G::receiveData() <ENTER>");
    if (sockFd < 0) return false;

    buffer.resize(2048); // Max UDP size
    sockaddr_in senderAddr{};
    socklen_t addrLen = sizeof(senderAddr);

    ssize_t received = recvfrom(sockFd, reinterpret_cast<char*>(buffer.data()), buffer.size(), 0, reinterpret_cast<sockaddr*>(&senderAddr), &addrLen);
    if (received <= 0){
#ifdef _WIN32
        int errorCode = WSAGetLastError();
        LOG_ERROR("Recived failed Error Code: %d ",errorCode);
#else
        int errorCode = errno;
        LOG_TO_FILE("Failed to bind socket to IP:%s PORT:%d | Error Code: %d (%s)", localIp.c_str(), localPort, errorCode, strerror(errorCode));
#endif
        return false;
    }
    buffer.resize(received);
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &senderAddr.sin_addr, ipStr, sizeof(ipStr));
    senderIp = ipStr;
    senderPort = ntohs(senderAddr.sin_port);
    LOG_INFO("EthernetSocketPL1G::receiveData() <EXIT>");
    return true;
}

int EthernetSocketPL1G::receiveData(char* buffer, int len, std::string& senderIp, uint16_t& senderPort)
{
    //LOG_INFO("EthernetSocketPL1G::receiveData() <ENTER>");
    if (sockFd < 0 || buffer == nullptr || len <= 0) {
        LOG_ERROR("Invalid socket or buffer parameters");
        return false;
    }
    sockaddr_in senderAddr{};
    socklen_t addrLen = sizeof(senderAddr);
    ssize_t received = recvfrom(sockFd,buffer,2048,0,reinterpret_cast<sockaddr*>(&senderAddr),&addrLen);
    //Log::printHexRecvBuffer(buffer,16);
    if (received <= 0)
    {
#ifdef _WIN32
        int errorCode = WSAGetLastError();
        LOG_ERROR("Recived failed Error Code: %d ",errorCode);
#else
        int errorCode = errno;
        LOG_TO_FILE("Failed to bind socket to IP:%s PORT:%d | Error Code: %d (%s)", localIp.c_str(), localPort, errorCode, strerror(errorCode));
#endif
        return false;
    }

    char ipStr[INET_ADDRSTRLEN] = {};
    if (inet_ntop(AF_INET, &senderAddr.sin_addr, ipStr, sizeof(ipStr)) == nullptr)
    {
        LOG_ERROR("Recived IP Conversion failed %s",ipStr);
        return false;
    }
    senderIp = ipStr;
    senderPort = ntohs(senderAddr.sin_port);
    //LOG_TO_FILE("EthernetSocketPL1G::receiveData() <EXIT>");
    return received;
}
bool EthernetSocketPL1G::getConnStatus(){
    return IsConnected;
}
bool EthernetSocketPL1G::closeSocket()
{
    if (sockFd >= 0) {
#ifdef _WIN32
        closesocket(sockFd);
#else
        close(sockFd);
#endif
        sockFd = -1;
    }
    IsConnected = false;
    return true;
}

std::string EthernetSocketPL1G::getInterfaceLabel() const {
    return interfaceLabel;
}

void EthernetSocketPL1G::setInterfaceLabel(const std::string& label) {
    interfaceLabel = label;
}

EthernetSocketPL1G *EthernetSocketPL1G::getInstance()
{
    if (instance) {
        //LOG_TO_FILE("EthernetSocketPL1G::getInstance() <ENTER>");
    } else {
        LOG_TO_FILE("EthernetSocketPL1G::getInstance() instance null");
    }
    return instance;

}
EthernetSocketPL1G *EthernetSocketPL1G::Create(const std::string& localIp,uint16_t localPort,QString RemoteIP,quint16 Port)
{
    LOG_INFO("EthernetSocketPL1G::create() <ENTER>");
    if (instance == nullptr)
    {
        LOG_INFO("EthernetSocketPL1G::create() <ALLOCATING> IP:%s PORT:%d LPORT:%d",localIp.c_str(),Port,localPort);
        instance = new EthernetSocketPL1G();
        instance->setInterfaceLabel("ETHPL1G");
        instance->RemoteIP = RemoteIP;
        instance->Port = Port;
        if(!instance->bindSocket(localIp,Port))
        {
            delete instance;
            instance = nullptr;
            LOG_INFO("EthernetSocketPL1G::Create() <FAILED TO OPEN PORT>");
            return nullptr;
        }
        LOG_INFO("EthernetSocketPL1G::create() <PORT OPENED SUCCESSFULLY>");
    }
    LOG_INFO("EthernetSocketPL1G::create() <EXIT>");
    return instance;
}
void EthernetSocketPL1G::destroyInstance()
{
    instance->closeSocket();
    delete instance;
    instance = nullptr;
}
