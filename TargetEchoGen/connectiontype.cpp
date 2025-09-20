#include "connectiontype.h"
#include "ui_connectiontype.h"

ConnectionType::ConnectionType(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ConnectionType)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);

    ui->LeConnPS1GIP->setText("127.0.0.1");
    ui->LeConnPL1GIP->setText("10.0.0.123");
    ui->LeConn10GIP->setText("192.168.30.245");
    ui->LeConnPS1GPort->setText("12345");
    ui->LeConnPL1GPort->setText("12345");
    ui->LeConn10GPort->setText("12345");

    ui->LblConnPS1GStatusLed->setPixmap(QPixmap(":/images/led-circle-grey.png"));
    ui->LblConnPL1GStatusLed->setPixmap(QPixmap(":/images/led-circle-grey.png"));
    ui->LblConn10GStatusLed->setPixmap(QPixmap(":/images/led-circle-grey.png"));
    ui->LblConnPSUartStatusLed->setPixmap(QPixmap(":/images/led-circle-grey.png"));
    ui->LblConnPLUartStatusLed->setPixmap(QPixmap(":/images/led-circle-grey.png"));

    // Regular expression for IPv4
    QRegularExpression ipv4Regex("^((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])\\.){3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])$");
    QRegularExpressionValidator *ipv4Validator = new QRegularExpressionValidator(ipv4Regex, this);
    ui->LeConnPS1GIP->setValidator(ipv4Validator);
    ui->LeConnPL1GIP->setValidator(ipv4Validator);
    ui->LeConn10GIP->setValidator(ipv4Validator);
    //ui->PbConnPL1GConn->setFixedWidth(100);
    int maxWidth = std::max(
        ui->PbConnPL1GConn->fontMetrics().horizontalAdvance("Connect"),
        ui->PbConnPL1GConn->fontMetrics().horizontalAdvance("Disconnect")
        );
    ui->PbConnPL1GConn->setMinimumWidth(maxWidth + 20);

    QRegularExpression portregex("^([1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$");
    ui->LeConnPS1GIP->setValidator(new QRegularExpressionValidator(portregex));

}

ConnectionType::~ConnectionType()
{
    delete ui;
}

void ConnectionType::on_PbConnPS1GConn_clicked()
{
    LOG_INFO("ConnectionType::on_PbConnPS1GConn_clicked() <ENTER>");
    if(ui->PbConnPS1GConn->text() == "Connect")
    {
        QString TargetIP = ui->LeConnPL1GPort->text();
        _pEthPL1G = EthernetSocketPL1G::Create("0.0.0.0",0,TargetIP,ui->LeConnPL1GPort->text().toInt());
        if(_pEthPL1G != nullptr){
            ui->PbConnPS1GConn->setText("Disconnect");
            ui->LblConnPL1GStatusLed->setPixmap(QPixmap(":/images/led-green_icon.jpg"));
            emit connectionSucceeded(eETHPS1G);
        }
        else
        {
            ui->LblConnPL1GStatusLed->setPixmap(QPixmap(":/images/led-icon-red.jpg"));
            ui->PbConnPL1GConn->setText("Connect   ");
            emit connectionFailed(eETHPS1G);
        }
    }
    else
    {
        if(ui->PbConnPL1GConn->text() == "Disconnect")
        {
            EthernetSocket::destroyInstance();
            ui->LblConnPL1GStatusLed->setPixmap(QPixmap(":/images/led-icon-red.jpg"));
            ui->PbConnPL1GConn->setText("Connect   ");
            emit connectionFailed(eETHPS1G);
        }
    }
    LOG_INFO("ConnectionType::on_PbConnPS1GConn_clicked() <EXIT>");
}


void ConnectionType::on_PbConnPL1GConn_clicked()
{
    LOG_INFO("ConnectionType::on_PbConnPL1GConn_clicked() <ENTER>");
    if(ui->PbConnPL1GConn->text() == "Connect")
    {
        QString TargetIP = ui->LeConnPL1GPort->text();
        _pEthPL1G = EthernetSocketPL1G::Create("192.168.30.240",0,TargetIP,ui->LeConnPL1GPort->text().toInt());
        if(_pEthPL1G != nullptr){
            ui->PbConnPL1GConn->setText("Disconnect");
            ui->LblConnPL1GStatusLed->setPixmap(QPixmap(":/images/led-green_icon.jpg"));
            emit connectionSucceeded(eETHPL1G);
        }
        else
        {
            ui->LblConnPL1GStatusLed->setPixmap(QPixmap(":/images/led-icon-red.jpg"));
            ui->PbConnPL1GConn->setText("Connect");
            emit connectionFailed(eETHPL1G);
        }
    }
    else
    {
        if(ui->PbConnPL1GConn->text() == "Disconnect")
        {
            EthernetSocketPL1G::destroyInstance();
            ui->LblConnPL1GStatusLed->setPixmap(QPixmap(":/images/led-icon-red.jpg"));
            ui->PbConnPL1GConn->setText("Connect");
            emit connectionFailed(eETHPL1G);
        }
    }
    LOG_INFO("ConnectionType::on_PbConnPL1GConn_clicked() <EXIT>");
}

void ConnectionType::on_PbConn10GConn_clicked()
{
    LOG_INFO("ConnectionType::on_PbConn10GConn_clicked() <ENTER>");
    if(ui->PbConnPL1GConn->text() == "Connect")
    {
        QString TargetIP = ui->LeConnPL1GPort->text();
        _pEthPL10G = EthernetSocket10G::Create("192.168.30.240",0,TargetIP,ui->LeConnPL1GPort->text().toInt());
        if(_pEthPL10G != nullptr){
            ui->PbConn10GConn->setText("Disconnect");
            ui->LblConn10GStatusLed->setPixmap(QPixmap(":/images/led-green_icon.jpg"));
            emit connectionSucceeded(eETH10G);
        }
        else
        {
            ui->LblConnPL1GStatusLed->setPixmap(QPixmap(":/images/led-icon-red.jpg"));
            ui->PbConnPL1GConn->setText("Connect");
            emit connectionFailed(eETH10G);
        }
    }
    else
    {
        if(ui->PbConn10GConn->text() == "Disconnect")
        {
            EthernetSocket10G::destroyInstance();
            ui->LblConnPL1GStatusLed->setPixmap(QPixmap(":/images/led-icon-red.jpg"));
            ui->PbConnPL1GConn->setText("Connect");
            emit connectionFailed(eETH10G);
        }
    }
    LOG_INFO("ConnectionType::on_PbConnPL1GConn_clicked() <EXIT>");
}

