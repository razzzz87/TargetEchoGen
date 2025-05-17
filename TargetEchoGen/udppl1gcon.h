#ifndef UDPPL1GCON_H
#define UDPPL1GCON_H

#include <QObject>
#include <QUdpSocket>
#include <QCoreApplication>
#include <QDebug>
#include <QByteArray>
#include <QHostAddress>
#include <QTimer>
#include <log.h>

enum {
    NOT_CONNECTED_PL_1G,
    CONNECTED_PL_1G,
};
class UDP_PL1G_Con : public QObject
{
    Q_OBJECT

public:
    static UDP_PL1G_Con& getInstance() {
        static UDP_PL1G_Con instance;
        return instance;
    }

    void bindSocket(const QString &ipAddress, quint16 port);
    void sendMessage(const QString &message, const QString &ipAddress, quint16 port);
    void sendMessage(const char* message,int len, const QString &ipAddress, quint16 port);
    void setSocketBufferSize(int size);
    void startKeepAlive(const QString &ipAddress, quint16 port, int interval);
    bool getConStatus();
    void getUDPSockStatus(QUdpSocket *udpSocket);
    bool Disconnect();
private slots:
    void processPendingDatagrams() {
        while (udpSocket->hasPendingDatagrams()) {
            QByteArray buffer;
            buffer.resize(udpSocket->pendingDatagramSize());
            udpSocket->readDatagram(buffer.data(), buffer.size());
            Log::printHexCStyle(buffer);
        }
    }

    void sendKeepAlive() {
        QByteArray data = "KEEP_ALIVE";
        udpSocket->writeDatagram(data, QHostAddress(ipAddress), port);
        qDebug() << "Sent keep-alive message";
    }

private:
    UDP_PL1G_Con(QObject *parent = nullptr) : QObject(parent) {
        udpSocket = new QUdpSocket(this);
        keepAliveTimer = new QTimer(this);
        connect(keepAliveTimer, &QTimer::timeout, this, &UDP_PL1G_Con::sendKeepAlive);
        IsConnected = false;
    }

    ~UDP_PL1G_Con() = default;

    UDP_PL1G_Con(const UDP_PL1G_Con&) = delete;
    UDP_PL1G_Con& operator=(const UDP_PL1G_Con&) = delete;

    QUdpSocket *udpSocket;
    QTimer *keepAliveTimer;
    QString ipAddress;
    quint16 port;
    bool IsConnected;
};

#endif // UDPPL1GCON_H
