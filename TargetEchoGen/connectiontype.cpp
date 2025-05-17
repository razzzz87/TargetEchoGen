#include "connectiontype.h"
#include "ui_connectiontype.h"

ConnectionType::ConnectionType(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ConnectionType)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);

    ui->lineEdit_ps_1g_ip->setText("127.0.0.1");
    ui->lineEdit_pl_1g_ip->setText("10.0.0.123");
    ui->lineEdit_pl_10g_ip->setText("192.168.30.245");
    ui->lineEdit_ps_1g_port->setText("12345");

    ui->label_conn_ps_1g_led->setPixmap(QPixmap(":/images/led-circle-grey.png"));
    ui->label_conn_pl_1g_led->setPixmap(QPixmap(":/images/led-circle-grey.png"));
    ui->label_conn_pl_10g_led->setPixmap(QPixmap(":/images/led-circle-grey.png"));
    ui->label_con_ps_uart_led->setPixmap(QPixmap(":/images/led-circle-grey.png"));
    ui->label_conn_pl_uart_led->setPixmap(QPixmap(":/images/led-circle-grey.png"));

    // Regular expression for IPv4
    QRegularExpression ipv4Regex("^((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])\\.){3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])$");
    QRegularExpressionValidator *ipv4Validator = new QRegularExpressionValidator(ipv4Regex, this);
    ui->lineEdit_ps_1g_ip->setValidator(ipv4Validator);
    ui->lineEdit_pl_1g_ip->setValidator(ipv4Validator);
    ui->lineEdit_pl_10g_ip->setValidator(ipv4Validator);

    QRegularExpression portregex("^([1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$");
    ui->lineEdit_ps_1g_port->setValidator(new QRegularExpressionValidator(portregex));
    UDPPS_01G_Con = &UDP_PS1G_Con::getInstance();
    UDPPL_10G_Con = &UDP_PL10G_Con::getInstance();
    UDPPL_01G_Con = &UDP_PL1G_Con::getInstance();

}

ConnectionType::~ConnectionType()
{
    delete ui;
}

void ConnectionType::on_pushButton_connect_pl_1g_clicked()
{
    LOG_TO_FILE(":Entry:\n");
    if(ui->pushButton_connect_pl_1g->text() == "Connect"){

        QString ip = ui->lineEdit_pl_1g_ip->text();
        LOG_TO_FILE("PS 1G IP Address:%s\n",ip.toStdString().c_str());
        UDPPL_01G_Con->bindSocket(ip,000);
        if(UDPPL_01G_Con->getConStatus() == NOT_CONNECTED){
            LOG_TO_FILE("ERROR: Con Failed : PS 1G IP Address:%s\n",ip.toStdString().c_str());
            ui->label_conn_pl_1g_led->setPixmap(QPixmap(":/images/led-icon-red.jpg"));
            ui->pushButton_connect_pl_1g->setText("Connect");
        }else{
            ui->pushButton_connect_pl_1g->setText("Disconnect");
            ui->label_conn_ps_1g_led->setPixmap(QPixmap(":/images/led-green_icon.jpg"));
            LOG_TO_FILE("UDP Connection succeeded\n");
        }
    }else{
        if(ui->pushButton_connect_pl_1g->text() == "Disconnect")
        {
            if(UDPPL_01G_Con->Disconnect()){
                ui->pushButton_connect_pl_1g->setText("Connect");
                qDebug() << "Diconnection done: IP:";
                ui->label_conn_pl_1g_led->setPixmap(QPixmap(":/images/led-icon-red.jpg"));
                LOG_TO_FILE("UDP Connection closed successfully\n");
            }else{
                ui->pushButton_connect_pl_1g->setText("Disconnect");
                LOG_TO_FILE("ERROR: Disconnection failed\n");
            }
        }
    }
    LOG_TO_FILE(":Exit:\n");
}


void ConnectionType::on_pushButton_connect_ps1g_clicked()
{
    LOG_TO_FILE(":Entry:\n");
    if(ui->pushButton_connect_ps1g->text() == "Connect"){

        QString ip = ui->lineEdit_ps_1g_ip->text();
        qint16 port = ui->lineEdit_ps_1g_port->text().toInt();
        LOG_TO_FILE("PS 1G IP Address:%s Port: %d\n",ip.toStdString().c_str(),port);
        qDebug() << "IP:"<<ip << " port " << port;
        UDPPS_01G_Con->bindSocket(ip,port);
        if(UDPPS_01G_Con->getConStatus() == NOT_CONNECTED){
            qDebug() << "Failed IP:"<<ip << " port " << port;
            LOG_TO_FILE("ERROR: Con Failed : PS 1G IP Address:%s Port: %d\n",ip.toStdString().c_str(),port);
            ui->label_conn_ps_1g_led->setPixmap(QPixmap(":/images/led-icon-red.jpg"));
            ui->pushButton_connect_ps1g->setText("Connect");
        }else{
            ui->pushButton_connect_ps1g->setText("Disconnect");
            ui->label_conn_ps_1g_led->setPixmap(QPixmap(":/images/led-green_icon.jpg"));
            LOG_TO_FILE("UDP Connection succeeded\n");
        }
    }else{
        if(ui->pushButton_connect_ps1g->text() == "Disconnect")
        {
            if(UDPPS_01G_Con->Disconnect()){
                ui->pushButton_connect_ps1g->setText("Connect");
                qDebug() << "Diconnection done: IP:";
                ui->label_conn_ps_1g_led->setPixmap(QPixmap(":/images/led-icon-red.jpg"));
                LOG_TO_FILE("UDP Connection closed successfully\n");
            }else{
                ui->pushButton_connect_ps1g->setText("Disconnect");
                LOG_TO_FILE("ERROR: Disconnection failed\n");
            }
        }
    }
    LOG_TO_FILE(":Exit:\n");
}


void ConnectionType::on_pushButton_connect_pl_10g_clicked()
{
    LOG_TO_FILE(":Entry\n");
    if(ui->pushButton_connect_pl_10g->text() == "Connect"){

        QString ip = ui->lineEdit_pl_10g_ip->text();
        LOG_TO_FILE("PS 1G IP Address:%s\n",ip.toStdString().c_str());
        UDPPL_10G_Con->bindSocket(ip,000);
        if(UDPPL_10G_Con->getConStatus() == NOT_CONNECTED){
            LOG_TO_FILE("ERROR: Con Failed : PS 1G IP Address:%s\n",ip.toStdString().c_str());
            ui->label_conn_pl_10g_led->setPixmap(QPixmap(":/images/led-icon-red.jpg"));
            ui->pushButton_connect_pl_10g->setText("Connect");
        }else{
            ui->pushButton_connect_pl_10g->setText("Disconnect");
            ui->label_conn_pl_10g_led->setPixmap(QPixmap(":/images/led-green_icon.jpg"));
            LOG_TO_FILE("UDP Connection succeeded\n");
        }
    }else{
        if(ui->pushButton_connect_pl_10g->text() == "Disconnect")
        {
            if(UDPPL_01G_Con->Disconnect()){
                ui->pushButton_connect_pl_10g->setText("Connect");
                qDebug() << "Diconnection done: IP:";
                ui->label_conn_pl_10g_led->setPixmap(QPixmap(":/images/led-icon-red.jpg"));
                LOG_TO_FILE("UDP Connection closed successfully\n");
            }else{
                ui->pushButton_connect_pl_10g->setText("Disconnect");
                LOG_TO_FILE("ERROR: Disconnection failed\n");
            }
        }
    }
    LOG_TO_FILE(":Exit:\n");
}


void ConnectionType::on_pushButton_conn_ps_uart_clicked()
{

}

