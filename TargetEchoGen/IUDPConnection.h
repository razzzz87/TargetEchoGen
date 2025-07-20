#ifndef IUDPCONNECTION_H
#define IUDPCONNECTION_H
#include <QHostAddress>

class IUDPConnection {
public:
    virtual int sendMessage(const QString &message, const QString &ipAddress, quint16 port) = 0;
    virtual int readResponsPacket(char* recvdata, int len, QHostAddress ipAddress, quint16 port) = 0;
    virtual bool getConStatus() const = 0;
    virtual ~IUDPConnection() = default;
};

#endif // IUDPCONNECTION_H
