#include "ethernetsocket.h"
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

EthernetSocket *EthernetSocket::instance = nullptr;

EthernetSocket::EthernetSocket() : sockFd(-1) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    IsConnected = false;
}

EthernetSocket::~EthernetSocket() {
    closeSocket();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool EthernetSocket::bindSocket(const std::string& localIp, uint16_t localPort) {

    LOG_TO_FILE("EthernetSocket::bindSocket() <ENTER>");
    sockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockFd < 0) {
        std::cerr << "Failed to create socket\n";
        return false;
    }

    sockaddr_in localAddr{};
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(localPort);
    localAddr.sin_addr.s_addr = inet_addr(localIp.c_str());
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
    LOG_TO_FILE("EthernetSocket::bindSocket() <EXIT>");
    IsConnected = true;
    return IsConnected;
}

bool EthernetSocket::sendData(const std::vector<uint8_t>& data,
                                     const std::string& destIp, uint16_t destPort) {
    LOG_TO_FILE("EthernetSocket::sendData <ENTER>");
    if (sockFd < 0) return false;

    sockaddr_in destAddr{};
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(destPort);
    destAddr.sin_addr.s_addr = inet_addr(destIp.c_str());

    ssize_t sent = sendto(sockFd, reinterpret_cast<const char*>(data.data()), data.size(), 0,
                          reinterpret_cast<sockaddr*>(&destAddr), sizeof(destAddr));
    LOG_TO_FILE("EthernetSocket::sendData <EXIT>");
    return sent == static_cast<ssize_t>(data.size());
}

bool EthernetSocket::sendData(const char* data, int datalen, const std::string& destIp, uint16_t destPort)
{
    LOG_TO_FILE("EthernetSocket::sendData() <ENTER>");

    if (sockFd < 0 || data == nullptr || datalen <= 0) {
        LOG_TO_FILE("Invalid socket or data parameters. sockFd=%d, datalen=%d", sockFd, datalen);
        return false;
    }

    sockaddr_in destAddr{};
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(destPort);

    LOG_TO_FILE("Preparing to send to IP: %s, Port: %d, DataLen: %d", destIp.c_str(), destPort, datalen);

    if (inet_pton(AF_INET, destIp.c_str(), &destAddr.sin_addr) <= 0) {
#ifdef _WIN32
        int errorCode = WSAGetLastError();
        LOG_TO_FILE("Invalid destination IP address: %s | Error Code: %d", destIp.c_str(), errorCode);
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
        LOG_TO_FILE("sendto() failed | Error Code: %d", errorCode);
#else
        int errorCode = errno;
        LOG_TO_FILE("sendto() failed | Error Code: %d (%s)", errorCode, strerror(errorCode));
#endif
        return false;
    }

    LOG_TO_FILE("Sent %zd bytes to %s:%d", sent, destIp.c_str(), destPort);
    LOG_TO_FILE("EthernetSocket::sendData() <EXIT>");
    return sent == datalen;
}

bool EthernetSocket::receiveData(std::vector<uint8_t>& buffer,
                                        std::string& senderIp, uint16_t& senderPort) {
    LOG_TO_FILE("EthernetSocket::receiveData() <ENTER>");
    if (sockFd < 0) return false;

    buffer.resize(2048); // Max UDP size
    sockaddr_in senderAddr{};
    socklen_t addrLen = sizeof(senderAddr);

    ssize_t received = recvfrom(sockFd, reinterpret_cast<char*>(buffer.data()), buffer.size(), 0, reinterpret_cast<sockaddr*>(&senderAddr), &addrLen);
    if (received <= 0){
#ifdef _WIN32
        int errorCode = WSAGetLastError();
        LOG_TO_FILE("Recived failed Error Code: %d ",errorCode);
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
    LOG_TO_FILE("EthernetSocket::receiveData() <EXIT>");
    return true;
}

bool EthernetSocket::receiveData(char* buffer, int len, std::string& senderIp, uint16_t& senderPort)
{
    LOG_TO_FILE("Char* EthernetSocket::receiveData() <ENTER>");
    if (sockFd < 0 || buffer == nullptr || len <= 0) {
        std::cerr << "Invalid socket or buffer parameters.\n";
        return false;
    }

    sockaddr_in senderAddr{};
    socklen_t addrLen = sizeof(senderAddr);
    LOG_TO_FILE("LEN:%d",len);
    ssize_t received = recvfrom(sockFd,buffer,len,0,reinterpret_cast<sockaddr*>(&senderAddr),&addrLen);
    LOG_TO_FILE("received:%d",received);
    if (received <= 0)
    {
#ifdef _WIN32
        int errorCode = WSAGetLastError();
        LOG_TO_FILE("Recived failed Error Code: %d ",errorCode);
#else
        int errorCode = errno;
        LOG_TO_FILE("Failed to bind socket to IP:%s PORT:%d | Error Code: %d (%s)", localIp.c_str(), localPort, errorCode, strerror(errorCode));
#endif
        return false;
    }

    char ipStr[INET_ADDRSTRLEN] = {};
    if (inet_ntop(AF_INET, &senderAddr.sin_addr, ipStr, sizeof(ipStr)) == nullptr)
    {
        LOG_TO_FILE("Recived IP Conversion failed %s",ipStr);
        return false;
    }
    senderIp = ipStr;
    senderPort = ntohs(senderAddr.sin_port);
    LOG_TO_FILE("Char* EthernetSocket::receiveData() <EXIT>");
    return true;
}
bool EthernetSocket::getConnStatus(){
    return IsConnected;
}
bool EthernetSocket::closeSocket()
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

std::string EthernetSocket::getInterfaceLabel() const {
    return interfaceLabel;
}

void EthernetSocket::setInterfaceLabel(const std::string& label) {
    interfaceLabel = label;
}

EthernetSocket *EthernetSocket::getInstance()
{
    if (instance) {
        LOG_TO_FILE("EthernetSocket::getInstance() <ENTER>");
    } else {
        LOG_TO_FILE("EthernetSocket::getInstance() instance null");
    }
    return instance;

}
EthernetSocket *EthernetSocket::Create(const std::string& localIp,uint16_t localPort,QString RemoteIP,quint16 Port)
{
    LOG_TO_FILE("UartSerial::create() <ENTER>");
    if (instance == nullptr)
    {
        LOG_TO_FILE("UartSerial::create() <ALLOCATING> IP:%s PORT:%d LPORT:%d",localIp.c_str(),Port,localPort);
        instance = new EthernetSocket();
        instance->setInterfaceLabel("ETH1G");
        instance->RemoteIP = RemoteIP;
        instance->Port = Port;
        if(!instance->bindSocket(localIp,Port))
        {
            delete instance;
            instance = nullptr;
             LOG_TO_FILE("EthernetSocket::Create() <FAILED TO OPEN PORT>");
            return nullptr;
        }
        LOG_TO_FILE("UartSerial::create() <PORT OPENED SUCCESSFULLY>");
    }
    LOG_TO_FILE("UartSerial::create() <EXIT>");
    return instance;
}
void EthernetSocket::destroyInstance()
{
    instance->closeSocket();
    delete instance;
    instance = nullptr;
}
