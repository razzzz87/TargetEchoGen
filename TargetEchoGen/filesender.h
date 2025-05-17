#ifndef FILESENDER_H
#define FILESENDER_H

#include <QObject>
#include <QCoreApplication>
#include <QUdpSocket>
#include <QFile>
#include <QThread>
#include <QDebug>
#include <QHostAddress>
#include <QMutex>

class FileSender : public QThread
{
    Q_OBJECT

public:
    FileSender(QUdpSocket *udpSocket, const QString &filePath, const QString &ipAddress, quint16 port, int readSize, QObject *parent = nullptr)
        : QThread(parent), udpSocket(udpSocket), filePath(filePath), ipAddress(ipAddress), port(port), readSize(readSize) {}

    void setReadSize(int newSize);
    void setFileSize(QString filePath);
    void abortFileWrite(bool abort=false);

protected:
    void run();
private:
    QUdpSocket *udpSocket;
    QString filePath;
    QString ipAddress;
    quint16 port;
    int readSize;
    QMutex mutex;
    bool abort;
};

#endif // FILESENDER_H
