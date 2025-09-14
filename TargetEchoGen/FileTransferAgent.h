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
#include "ethernetsocket.h"
#include "uartserial.h"
#include "ethernetsocket10G.h"
#include "Utils.h"

#define AFTER_NUMBER_OF_PKT 10
#define MAX_BYTES_WRITE_AT_ONCE			    1456// 1024// 16 * 1024 + 12// 1024// 16384//1448//16384
#define MAX_BULK_READ_DATA_SIZE			    1048576// 64512//32768//16384// 8192// 4096//  1448// 1024//16 * 1024 - 12

class FileTransferAgent  : public QThread
{
    Q_OBJECT

public:

    explicit FileTransferAgent(QObject* parent = nullptr)
        : QThread(parent), eth0(nullptr), eth10G(nullptr),
        _Port(0), _iDataSize(0), abort(false), bytesTransferred(0) { abort = false;}

    //Configuration function
    void configure(const QString& ip, quint16 portNum, const QString& path, int chunkSize,eXferDir dir);
    void configure(stFileReadWriteConf stReadWriteCnf);

    void setupDevice(iface deviceType);
    int GetTotalbyte();
    int GetTransferBbyte();

    void setupProgressTimer();
    int ReadFileBulkEth01G(unsigned int startAddress, qint64* numBytesRdSuccess);
    int ReadFileBulkEth10G(unsigned int startAddress, qint64* numBytesRdSuccess);
    int StreamReadFileBulkEth10G(unsigned int startAddress, qint64* numBytesRdSuccess);
    int receiveFileBulkPL10G(unsigned int startAddress, qint64* numBytesRdSuccess);
    int sendFileBulkPS01G(unsigned int startAddress, unsigned int size);
    int receiveFileBulkEth10G(unsigned startAddress,qint64* numBytesRdSuccess);
    int sendFileBulkPL01G(unsigned int startAddress, unsigned int size, const QString& filePath);
    int WriteFileBulk10G(unsigned int startAddress, qint64* numBytesRdSuccess);

public slots:
        void abortTransfer() {
        QMutexLocker locker(&mutex);
            abort = true;
    }

signals:
    void progressUpdated(qint64 bytesTransferred);
    void transferComplete();

protected:
    void run() override;

private:

    EthernetSocket *eth0;
    EthernetSocket10G *eth10G;
    iface _eInterFace;
    TransferMode TransferMode;
    eXferDir _Dir;

    QString _sFilePath;
    QString _IPAddress;
    quint16 _Port;
    int _iDataSize;

    QMutex mutex;
    bool abort;

    qint64 bytesTransferred;
    unsigned int TransferReqSize;
};
#endif // FILESENDER_H
