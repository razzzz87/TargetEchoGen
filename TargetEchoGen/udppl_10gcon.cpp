#include "udppl_10gcon.h"

void UDP_PL10G_Con::bindSocket(const QString &ipAddress, quint16 port) {

    bool success = udpSocket->bind(QHostAddress(ipAddress), port);
    if (success) {
        LOG_TO_FILE("Socket successfully bind!");
        IsConnected = true;
    } else {
        LOG_TO_FILE("Failed to bind socket.");
        IsConnected = false;
        return;
    }
    IsConnected = true;
    getUDPSockStatus(udpSocket); //status in log
    connect(udpSocket, &QUdpSocket::readyRead, this, &UDP_PL10G_Con::processPendingDatagrams);
}
bool UDP_PL10G_Con::Disconnect() {
    LOG_TO_FILE(":Entry==>");
    udpSocket->close();
    if (udpSocket->state() == QAbstractSocket::UnconnectedState) {
        LOG_TO_FILE("Socket successfully closed.");
        return true;
    } else {
        LOG_TO_FILE("Socket is still connected error:%s",udpSocket->error());
        return false;
    }
}
void UDP_PL10G_Con ::sendMessage(const char* message,int len, const QString &ipAddress, quint16 port)
{
    LOG_TO_FILE(":Entry==>");
    qint64 bytesWritten = udpSocket->writeDatagram(message,len,QHostAddress(ipAddress), port);
    if (bytesWritten == -1) {
        LOG_TO_FILE("Failed to send datagram.");
    } else {
        LOG_TO_FILE("Bytes written: %ld ",bytesWritten);
    }
    LOG_TO_FILE(":Exit==>");
}
void UDP_PL10G_Con::sendMessage(const QString &message, const QString &ipAddress, quint16 port)
{
    QByteArray data = message.toUtf8();
    qint64 bytesWritten = udpSocket->writeDatagram(data, QHostAddress(ipAddress), port);
    if (bytesWritten == -1) {
        LOG_TO_FILE("Failed to send datagram.");
    } else {
        LOG_TO_FILE("Datagram sent successfully. Bytes written: %ld ",bytesWritten);
    }
}

void UDP_PL10G_Con::setSocketBufferSize(int size) {

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
        LOG_TO_FILE("Socket option set successfully.");    }
}

void UDP_PL10G_Con::startKeepAlive(const QString &ipAddress, quint16 port, int interval) {
    this->ipAddress = ipAddress;
    this->port = port;
    keepAliveTimer->start(interval);
}
bool UDP_PL10G_Con::getConStatus(){
    return IsConnected;
}
void UDP_PL10G_Con::getUDPSockStatus(QUdpSocket *udpSocket)
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

