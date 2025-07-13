#include "udpcon.h"
#include <QUdpSocket>          // For UDP communication
#include <QHostAddress>        // For specifying IP addresses
#include <QNetworkInterface>   // To enumerate host IPs (if needed)

void UDP_PS1G_Con ::bindSocket(qint16 port) {
    LOG_TO_FILE(":Entry==>");
    udpSocket->error();

    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol &&
            !address.isLoopback()) {
            qDebug() << "Detected Host IP:" << address.toString();
            LOG_TO_FILE("HOSTIP: %s",address.toString().toStdString().c_str());
            this->host_ip = address.toString();
        }
    }
    bool success = udpSocket->bind(QHostAddress(this->host_ip), port);
    if (success) {
        LOG_TO_FILE("Socket successfully bind!");
        IsConnected = true;
    } else {
        LOG_TO_FILE("Failed to bind socket.");
        qDebug() << "Sock failed";
        IsConnected = false;
    }
    getUDPSockStatus(udpSocket); //status in log
    LOG_TO_FILE(":Exit==>");
    return;
}

bool UDP_PS1G_Con ::Disconnect() {
    LOG_TO_FILE(":Entry==>");
    udpSocket->close();
    if (udpSocket->state() == QAbstractSocket::UnconnectedState) {
        LOG_TO_FILE("Socket successfully closed.");
        return true;
    } else {
        LOG_TO_FILE("Socket is still connected error:%s",udpSocket->error());
        return false;
    }
    LOG_TO_FILE(":Exit==>");
}

int UDP_PS1G_Con::sendMessage(const char* message,int len, const QString &ipAddress, quint16 port)
{
    LOG_TO_FILE(":Entry==>");
    qint64 bytesWritten = udpSocket->writeDatagram(message,len,QHostAddress(ipAddress), port);
    if (bytesWritten == -1) {
        LOG_TO_FILE("Failed to send datagram.");

    } else {
        LOG_TO_FILE("Bytes written: %ld ",bytesWritten);
    }
    LOG_TO_FILE(":Exit==>");
    return bytesWritten;
}
int UDP_PS1G_Con::readResponsPacket(char* recvdata,int len, QHostAddress ipAddress, quint16 port)
{
    LOG_TO_FILE(":Entry==>");
    qint64 bytesRead = -1;
    if(udpSocket->waitForReadyRead()){
        if(udpSocket->hasPendingDatagrams()){
            bytesRead = udpSocket->readDatagram(recvdata,len,&ipAddress,&port);
            if (bytesRead == -1) {
                LOG_TO_FILE("Failed to send datagram.");
            } else {
                LOG_TO_FILE("Bytes Read: %ld ",bytesRead);
                Log::printHexRecvBuffer(recvdata,len);
            }
        }
    }else{
        LOG_TO_FILE("Timeout waiting for datagram.");
    }
    LOG_TO_FILE(":Exit==>");
    return bytesRead;
}


void UDP_PS1G_Con::sendMessage(const QString &message, const QString &ipAddress, quint16 port)
{
    LOG_TO_FILE(":Entry==>");
    QByteArray data = message.toUtf8();
    qint64 bytesWritten = udpSocket->writeDatagram(data, QHostAddress(ipAddress), port);
    if (bytesWritten == -1) {
        LOG_TO_FILE("Failed to send datagram.");
    } else {
        LOG_TO_FILE("Datagram sent successfully. Bytes written: %ld ",bytesWritten);
    }
    LOG_TO_FILE(":Exit==>");
}


void UDP_PS1G_Con ::setSocketBufferSize(int size) {

    LOG_TO_FILE(":Entry==>");
    udpSocket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, size);
    if (udpSocket->error() != QAbstractSocket::UnknownSocketError) {
        LOG_TO_FILE("Failed to set socket option:%s",udpSocket->errorString().toStdString().c_str());
    } else {
        LOG_TO_FILE("Socket option set successfully.");
    }
    udpSocket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, size);
    if (udpSocket->error() != QAbstractSocket::UnknownSocketError) {
        LOG_TO_FILE("Failed to set socket option:%s",udpSocket->errorString().toStdString().c_str());
    } else {
        LOG_TO_FILE("Socket option set successfully.");
    }
    LOG_TO_FILE(":Exit==>");
}

void UDP_PS1G_Con ::startKeepAlive(const QString &ipAddress, quint16 port, int interval) {
    LOG_TO_FILE(":Entry==>");
    this->remote_ip = ipAddress;
    this->remote_port = port;
    keepAliveTimer->start(interval);
    LOG_TO_FILE(":Exit==>");
}
bool UDP_PS1G_Con ::getConStatus(){
    return IsConnected;
}
void UDP_PS1G_Con ::getUDPSockStatus(QUdpSocket *udpSocket)
{
    LOG_TO_FILE(":Entry==>");
    QAbstractSocket::SocketState state = udpSocket->state();
    //IsConnected = false;
    switch (state) {
    case QAbstractSocket::UnconnectedState:
        LOG_TO_FILE("UnconnectedState");
        break;
    case QAbstractSocket::HostLookupState:
        LOG_TO_FILE("HostLookupState");
        break;
    case QAbstractSocket::ConnectingState:
        LOG_TO_FILE("ConnectingState");
        break;
    case QAbstractSocket::ConnectedState:
        LOG_TO_FILE("Socket is connected.");
        IsConnected=true;
        break;
    case QAbstractSocket::BoundState:
        LOG_TO_FILE("BoundState");
        break;
    case QAbstractSocket::ClosingState:
        IsConnected=false;
        LOG_TO_FILE("Socket is about to close.");
        break;
    case QAbstractSocket::ListeningState:
        LOG_TO_FILE("Socket is in listening state.");
        break;
    default:
        LOG_TO_FILE("Unknown socket state.");
        break;
    }
    LOG_TO_FILE(":Exit==>");
}
