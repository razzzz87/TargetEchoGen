#ifndef DEVICESETUP_H
#define DEVICESETUP_H

#include <QWidget>
#include "Proto.h"
#include "udpcon.h"
#include "udppl1gcon.h"
#include "udppl_10gcon.h"
#include <QLineEdit>
#include "FileTransferAgent.h"
#include <QFileDialog>
#include "transferprogressdialog.h"

namespace Ui {
class DeviceSetup;
}

class DeviceSetup : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceSetup(QWidget *parent = nullptr);
    ~DeviceSetup();
    uint64_t ParseRegReadResponsePkt(char *rcvpkt, int pktLen);
    void readRegisterValue(QLineEdit* lineEditAddr, QLineEdit* lineEditVal);
    void WriteRegisterValue(QLineEdit* lineEditAddr, QLineEdit* lineEditVal);
    void FileReadWriteSetup(iface deviceType, uint iFileSize, QString sFilePath, eXferDir dir);
    TransferProgressDialog* progressDialog;
    FileTransferAgent  *setupTransferAgent;
    UartSerial *_pSerial;
    EthernetSocket *_pEthPS1G;
    EthernetSocketPL1G *_pEthPL1G;
    EthernetSocket10G *_pEth10G;
private slots:
    void updateTransferProgress(qint64 percentage);
    void close_Progress_pop(void);
    void on_PbRegRead1_clicked();
    void on_PbRegWrite1_clicked();

    void on_PbMemReadFileNameBrowse_clicked();

    void on_PbMemReadRead_clicked();

    void on_PbMemWriteFileBrowse_clicked();

    void on_PbMemWrite_clicked();

private:
    Ui::DeviceSetup *ui;
    Proto objProto;
    QHostAddress ipAddress;
    quint16 port;
    bool transferCanceled = false;
};

#endif // DEVICESETUP_H
