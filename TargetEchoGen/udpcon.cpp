#include "udpcon.h"

void UDP_PS1G_Con ::bindSocket(const QString &ipAddress, quint16 port) {
    LOG_TO_FILE(":Entry==>");
    udpSocket->error();
    bool success = udpSocket->bind(QHostAddress(ipAddress), port);
    if (success) {
        LOG_TO_FILE("Socket successfully bound!\n");
        IsConnected = true;
    } else {
        LOG_TO_FILE("Failed to bind socket.\n");
        qDebug() << "Sock failed";
        IsConnected = false;
    }
    getUDPSockStatus(udpSocket); //status in log
    LOG_TO_FILE(":Exit:\n");
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
    LOG_TO_FILE(":Exit\n");
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
    LOG_TO_FILE(":Exit:\n");
    return bytesWritten;
}
int UDP_PS1G_Con::readResponsPacket(char* recvdata,int len, QHostAddress &ipAddress, quint16 port)
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
    }
    LOG_TO_FILE(":Exit:\n");
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
    LOG_TO_FILE(":Exit:\n");
}

void UDP_PS1G_Con ::setSocketBufferSize(int size) {

    LOG_TO_FILE(":Entry==>");
    udpSocket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, size);
    if (udpSocket->error() != QAbstractSocket::UnknownSocketError) {
        LOG_TO_FILE("Failed to set socket option:%s\n",udpSocket->errorString().toStdString().c_str());
    } else {
        LOG_TO_FILE("Socket option set successfully.\n");
    }
    udpSocket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, size);
    if (udpSocket->error() != QAbstractSocket::UnknownSocketError) {
        LOG_TO_FILE("Failed to set socket option:%s\n",udpSocket->errorString().toStdString().c_str());
    } else {
        LOG_TO_FILE("Socket option set successfully.\n");
    }
    LOG_TO_FILE(":Exit:\n");
}

void UDP_PS1G_Con ::startKeepAlive(const QString &ipAddress, quint16 port, int interval) {
    LOG_TO_FILE(":Entry==>");
    this->ipAddress = ipAddress;
    this->port = port;
    keepAliveTimer->start(interval);
    LOG_TO_FILE(":Exit:\n");
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
        LOG_TO_FILE("Socket is not connected.");
        qDebug() << "Socket is not connected.";
        break;
    case QAbstractSocket::HostLookupState:
        LOG_TO_FILE("Socket is performing a host name lookup.");
         qDebug() << "Socket is performing a host name lookup.";
        break;
    case QAbstractSocket::ConnectingState:
        LOG_TO_FILE("Socket is attempting to connect.");
        qDebug() << "Socket is performing a host name lookup.2";
        break;
    case QAbstractSocket::ConnectedState:
        LOG_TO_FILE("Socket is connected.");
        qDebug() << "Socket is performing a host name lookup.3";
        IsConnected=true;
        break;
    case QAbstractSocket::BoundState:
        LOG_TO_FILE("Socket is bound to an address and port.");
        qDebug() << "Socket is performing a host name lookup.4";
        break;
    case QAbstractSocket::ClosingState:
        IsConnected=false;
        LOG_TO_FILE("Socket is about to close.");
        qDebug() << "Socket is performing a host name lookup.5";
        break;
    case QAbstractSocket::ListeningState:
        LOG_TO_FILE("Socket is in listening state.");
        qDebug() << "Socket is performing a host name lookup.6";
        break;
    default:
        LOG_TO_FILE("Unknown socket state.");
        qDebug() << "Socket is performing a host name lookup.7";
        break;
    }
    LOG_TO_FILE(":Exit:\n");
}
