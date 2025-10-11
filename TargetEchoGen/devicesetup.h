#ifndef DEVICESETUP_H
#define DEVICESETUP_H

#include <QWidget>
#include "proto.h"
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
    void ReadRegisterAndShow(QLineEdit *leAddr, QLineEdit *leVal, const QString &devLabel, uint8_t dacPage = 0x00);
    void WriteRegisterAndShow(QLineEdit *leAddr, QLineEdit *leVal);
    iface getSelectedDeviceType();
    void FileReadWriteSetup(iface deviceType, uint iFileSize, QString sFilePath, eXferDir dir);
    TransferProgressDialog* progressDialog;
    FileTransferAgent  *setupTransferAgent;

private slots:
    void on_PbRegRead1_clicked();
    void on_PbRegWrite1_clicked();
    void on_PbMemReadFileNameBrowse_clicked();
    void on_PbMemReadRead_clicked();
    void on_PbMemWriteFileBrowse_clicked();
    void on_PbMemWrite_clicked();
    void on_PbLMKInitDefault_clicked();

    void on_PbDACInitDefault_clicked();

    void on_PbRegRead2_clicked();

    void on_PbRegRead3_clicked();

    void on_PbRegRead4_clicked();

    void on_PbRegWrite2_clicked();

    void on_PbRegWrite3_clicked();

    void on_PbRegWrite4_clicked();

private:
    Ui::DeviceSetup *ui;
    Proto objProto;
    QHostAddress ipAddress;
    quint16 port;
    bool transferCanceled = false;
};

#endif // DEVICESETUP_H
