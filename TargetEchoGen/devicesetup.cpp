#include "devicesetup.h"
#include "ui_devicesetup.h"
#include <QMessageBox>
#include <QHostAddress>
#include "log.h"
#include "Proto.h"

DeviceSetup::DeviceSetup(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DeviceSetup)
{
    ui->setupUi(this);

    EthPs01G = &UDP_PS1G_Con::getInstance();
    EthPl10G = &UDP_PL10G_Con::getInstance();
    EthPl01G = &UDP_PL1G_Con::getInstance();

    QRegularExpression hexRegex("^(0x)?[0-9A-Fa-f]{1,8}$"); // max 8 hex digits
    QRegularExpressionValidator* hexValidator = new QRegularExpressionValidator(hexRegex, this);
    ui->lineEdit_reg_read_addr1->setValidator(hexValidator);
    ui->lineEdit_reg_read_addr2->setValidator(hexValidator);
    ui->lineEdit_reg_read_addr3->setValidator(hexValidator);
    ui->lineEdit_reg_read_addr4->setValidator(hexValidator);

    ui->lineEdit_reg_read_val1->setReadOnly(true);
    ui->lineEdit_reg_read_val2->setReadOnly(true);
    ui->lineEdit_reg_read_val3->setReadOnly(true);
    ui->lineEdit_reg_read_val4->setReadOnly(true);

    ui->lineEdit_wr_reg_val1->setValidator(hexValidator);
    ui->lineEdit_wr_reg_val2->setValidator(hexValidator);
    ui->lineEdit_wr_reg_val3->setValidator(hexValidator);
    ui->lineEdit_wr_reg_val4->setValidator(hexValidator);


    ui->lineEdit_reg_wr_addr1->setValidator(hexValidator);
    ui->lineEdit_reg_wr_addr2->setValidator(hexValidator);
    ui->lineEdit_reg_wr_addr3->setValidator(hexValidator);
    ui->lineEdit_reg_wr_addr4->setValidator(hexValidator);



}

DeviceSetup::~DeviceSetup()
{
    delete ui;
}
uint64_t DeviceSetup::ParseRegReadResponsePkt(char* rcvpkt, int pktLen)
{
    char byArrPktResp[16];
    qint64 ret = -1;
    memcpy(byArrPktResp,rcvpkt,pktLen);
    Proto objProtoRespPkt;
    ret	= objProtoRespPkt.mParseResponsePkt(byArrPktResp);
    LOG_TO_FILE("Reg value:0x%X\n",ret);
    if(ret < 0){
        LOG_TO_FILE("Failed to parse packet\n");
    }else{
        LOG_TO_FILE("Packet parsing done:");
        if(objProtoRespPkt.mGetStatus() == 0){
            ret	=	objProtoRespPkt.mGetRegisterData();
            LOG_TO_FILE("Register value:%ld\n",ret);
        }
        else{
            LOG_TO_FILE("Failed to parse packet\n");
        }
    }
    return ret;
}
void DeviceSetup::on_pushButton_device_setup_reg1_read_clicked()
{
   readRegisterValue(ui->lineEdit_reg_read_addr1, ui->lineEdit_reg_read_val1);
}
void DeviceSetup::on_pushButton_device_setup_reg2_read_clicked()
{
    readRegisterValue(ui->lineEdit_reg_read_addr2, ui->lineEdit_reg_read_val2);
}
void DeviceSetup::on_pushButton_device_setup_reg3_read_clicked()
{
    readRegisterValue(ui->lineEdit_reg_read_addr3, ui->lineEdit_reg_read_val3);
}
void DeviceSetup::on_pushButton_device_setup_reg4_read_clicked()
{
    readRegisterValue(ui->lineEdit_reg_read_addr4, ui->lineEdit_reg_read_val4);
}

void DeviceSetup::readRegisterValue(QLineEdit* lineEditAddr, QLineEdit* lineEditVal) {

    LOG_TO_FILE(":Entry==>");
    char* byArrPkt = nullptr;
    char ByteArr64BitPakt[64];

    if (!EthPs01G->getConStatus()) {
        Log::showStatusMessage(this, "Device not connected", "Device not connected");
        return;
    }

    bool ok;
    uint addr = lineEditAddr->text().toUInt(&ok, 16);
    if (!ok) {
        LOG_TO_FILE("ERROR: Invalid address format.");
        return;
    }

    int pktLen = objProto.mPktRegRead(addr, &byArrPkt);
    if (pktLen == EthPs01G->sendMessage(byArrPkt, pktLen, EthPs01G->remote_ip, EthPs01G->remote_port)) {
        QHostAddress ip(EthPs01G->remote_ip);
        if (pktLen == EthPs01G->readResponsPacket(ByteArr64BitPakt, pktLen, ip, port)) {
            Proto objProtoRespPkt;
            int regval = objProtoRespPkt.mParseResponsePkt(ByteArr64BitPakt);
            LOG_ONLY_DATA("Reg VAl: 0x%X\n", regval);

            if (regval > 0) {
                lineEditVal->setText(QString("%1").arg(regval, 8, 16, QChar('0')).toUpper());
                LOG_TO_FILE("Value updated in lineEdit");
            }
        }
    } else {
        LOG_TO_FILE("ERROR: Unable to send data to udp socket.");
    }

    delete byArrPkt;
    LOG_TO_FILE(":Exit==>");
}

void DeviceSetup::WriteRegisterValue(QLineEdit* lineEditAddr, QLineEdit* lineEditVal) {

    LOG_TO_FILE(":Entry==>");
    char* byArrPkt = nullptr;
    char ByteArr64BitPakt[64];
    if (!EthPs01G->getConStatus()) {
        Log::showStatusMessage(this, "Device not connected", "Device not connected");
        return;
    }
    bool ok;
    uint addr = lineEditAddr->text().toUInt(&ok, 16);
    if (!ok) {
        LOG_TO_FILE("ERROR: Invalid address format.");
        return;
    }
    uint val = lineEditVal->text().toUInt(&ok, 16);
    if (!ok) {
        LOG_TO_FILE("ERROR: Invalid address format.");
        return;
    }
    int pktLen = objProto.mPktRegWrite(addr,val,&byArrPkt);
    if (pktLen == EthPs01G->sendMessage(byArrPkt, pktLen, EthPs01G->remote_ip, EthPs01G->remote_port)){

        QHostAddress ip(EthPs01G->remote_ip);
        //Read and flushed fd
        if (pktLen == EthPs01G->readResponsPacket(ByteArr64BitPakt, pktLen, ip, port)) {
        }
    } else {
        LOG_TO_FILE("ERROR: Unable to send data to udp socket.");
    }
    delete byArrPkt;
    LOG_TO_FILE(":Exit==>");
}
void DeviceSetup::on_pushButton_device_setup_wr_reg1_clicked()
{
    WriteRegisterValue(ui->lineEdit_reg_wr_addr1, ui->lineEdit_wr_reg_val1);
}
void DeviceSetup::on_pushButton_device_setup_wr_reg2_clicked()
{
    WriteRegisterValue(ui->lineEdit_reg_wr_addr2, ui->lineEdit_wr_reg_val2);
}
void DeviceSetup::on_pushButton_device_setup_wr_reg3_clicked()
{
    WriteRegisterValue(ui->lineEdit_reg_wr_addr3, ui->lineEdit_wr_reg_val3);
}
void DeviceSetup::on_pushButton_device_setup_wr_reg4_clicked()
{
    WriteRegisterValue(ui->lineEdit_reg_wr_addr4, ui->lineEdit_wr_reg_val4);
}

