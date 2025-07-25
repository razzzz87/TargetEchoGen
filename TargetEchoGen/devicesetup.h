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
private slots:
    void on_pushButton_device_setup_reg1_read_clicked();
    void on_pushButton_device_setup_reg2_read_clicked();
    void on_pushButton_device_setup_reg3_read_clicked();
    void on_pushButton_device_setup_reg4_read_clicked();

    void on_pushButton_device_setup_wr_reg1_clicked();
    void on_pushButton_device_setup_wr_reg2_clicked();
    void on_pushButton_device_setup_wr_reg3_clicked();
    void on_pushButton_device_setup_wr_reg4_clicked();

    void on_pushButton_devsetup_mem_read_start_clicked();
    void on_pushButton_device_setup_file_mem_read_clicked();

    void updateTransferProgress(qint64 percentage);
    void close_Progress_pop(void);

private:
    Ui::DeviceSetup *ui;
    UDP_PS1G_Con *EthPs01G;
    UDP_PL10G_Con *EthPl10G;
    UDP_PL1G_Con *EthPl01G;
    FileTransferAgent  *setupTransferAgent;
    Proto objProto;
    QHostAddress ipAddress;
    quint16 port;

    QProgressDialog* transferProgress = nullptr;
    bool transferCanceled = false;
};

#endif // DEVICESETUP_H
