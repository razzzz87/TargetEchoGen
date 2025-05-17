#ifndef DEVICESETUP_H
#define DEVICESETUP_H

#include <QWidget>
#include "Proto.h"
#include "udpcon.h"
#include "udppl1gcon.h"
#include "udppl_10gcon.h"

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
private slots:
    void on_pushButton_device_setup_reg_read_clicked();

private:
    Ui::DeviceSetup *ui;
    UDP_PS1G_Con *EthPs01G;
    UDP_PL10G_Con *EthPl10G;
    UDP_PL1G_Con *EthPl01G;
    Proto objProto;
    QHostAddress ipAddress;
    quint16 port;
};

#endif // DEVICESETUP_H
