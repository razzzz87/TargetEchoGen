#ifndef CONNECTIONTYPE_H
#define CONNECTIONTYPE_H
#include "udpcon.h"
#include "udppl_10gcon.h"
#include "udppl1gcon.h"
#include <QWidget>

namespace Ui {
class ConnectionType;
}

class ConnectionType : public QWidget
{
    Q_OBJECT

public:
    UDP_PS1G_Con *UDPPS_01G_Con;
    UDP_PL10G_Con *UDPPL_10G_Con;
    UDP_PL1G_Con  *UDPPL_01G_Con;
    explicit ConnectionType(QWidget *parent = nullptr);
    ~ConnectionType();

private slots: 
    void on_pushButton_connect_pl_1g_clicked();

    void on_pushButton_connect_ps1g_clicked();

    void on_pushButton_connect_pl_10g_clicked();

    void on_pushButton_conn_ps_uart_clicked();

private:
    Ui::ConnectionType *ui;
};

#endif // CONNECTIONTYPE_H
