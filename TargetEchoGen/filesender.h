#ifndef FILETRANSFERAGENT_H
#define FILETRANSFERAGENT_H

#include <QObject>
#include <QCoreApplication>
#include <QUdpSocket>
#include <QFile>
#include <QThread>
#include <QDebug>
#include <QHostAddress>
#include <QMutex>
#include <QThread>
#include <QUdpSocket>
#include <QMutex>
#include "Proto.h"
#include "udpcon.h"
#include "udppl1gcon.h"
#include "udppl_10gcon.h"

#define AFTER_NUMBER_OF_PKT 50
#define MAX_BYTES_WRITE_AT_ONCE			    9000// 1024// 16 * 1024 + 12// 1024// 16384//1448//16384
#define MAX_BULK_READ_DATA_SIZE			    1048576// 64512//32768//16384// 8192// 4096//  1448// 1024//16 * 1024 - 12

enum TransferMode { SendBulk, ReceiveBulk, Streaming };

enum TransferDirection {
    HostToTarget,   // Data sent from host to target
    TargetToHost    // Data received from target to host
};

class FileTransferAgent  : public QThread
{
    Q_OBJECT

public:
    enum DeviceType { PS1G, PL10G, PL1G };

    explicit FileTransferAgent(QObject* parent = nullptr)
        : QThread(parent), EthPs01G(nullptr), EthPl10G(nullptr), EthPl01G(nullptr),
        port(0), readSize(0), abort(false), bytesTransferred(0) {}

    //Configuration function
    void configure(const QString& ip, quint16 portNum, const QString& path, int chunkSize,TransferDirection dir);
    void setupDevice(UDP_PS1G_Con* ps1g);
    void setupDevice(UDP_PL10G_Con* pl10g);
    void setupDevice(UDP_PL1G_Con* pl1g);

    int GetTotalbyte();
    int GetTransferBbyte();

    void setupProgressTimer();
    void setReadSize(int newSize) { readSize = newSize; }
    void setFilePath(const QString& newFilePath) { filePath = newFilePath; }

    void abortFileWrite(bool abortFlag = false) {
        QMutexLocker locker(&mutex);
        abort = abortFlag;
    }

    int receiveFileBulkPS1G(unsigned int startAddress, qint64* numBytesRdSuccess);
    int receiveFileBulkPL01G(unsigned int startAddress, qint64* numBytesRdSuccess);
    int receiveFileBulkPL10G(unsigned int startAddress, qint64* numBytesRdSuccess);
    int sendFileBulkPS01G(unsigned int startAddress, unsigned int size);
    int sendFileBulkPL01G(unsigned int startAddress, unsigned int size, const QString& filePath);
    int sendFileBulkPL10G(unsigned int startAddress, unsigned int size, const QString& filePath);
signals:
    void progressUpdated(qint64 bytesTransferred);
    void close_progress_pop();

protected:
    void run() override;

private:
    UDP_PS1G_Con* EthPs01G;
    UDP_PL10G_Con* EthPl10G;
    UDP_PL1G_Con* EthPl01G;
    DeviceType deviceType;
    TransferMode TransferMode;
    TransferDirection dir;

    QString filePath;
    QString ipAddress;
    quint16 port;
    int readSize;

    QMutex mutex;
    bool abort;

    qint64 bytesTransferred;
    unsigned int TransferReqSize;
};
#endif // FILESENDER_H
