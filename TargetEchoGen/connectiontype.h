#ifndef CONNECTIONTYPE_H
#define CONNECTIONTYPE_H
#include "udpcon.h"
#include <QWidget>
#include "ethernetsocket.h"
#include "ethernetsocket10G.h"
#include "ethernetsocketpl1g.h"
#include "Utils.h"

namespace Ui {
class ConnectionType;
}

class ConnectionType : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionType(QWidget *parent = nullptr);
    ~ConnectionType();

private slots:
    void on_PbConnPS1GConn_clicked();
    void on_PbConnPL1GConn_clicked();
    void on_PbConn10GConn_clicked();

signals:
    void connectionSucceeded(iface eInterface);
    void connectionFailed(iface eInterface);

private:
    Ui::ConnectionType *ui;
    EthernetSocket *_pEthPS1G;
    EthernetSocketPL1G *_pEthPL1G;
    EthernetSocket10G *_pEthPL10G;

};

#endif // CONNECTIONTYPE_H
