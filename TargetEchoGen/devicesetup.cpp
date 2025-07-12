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
void DeviceSetup::on_pushButton_device_setup_reg_read_clicked()
{
    LOG_TO_FILE(":Entry==>");
    //char *byArrPkt  =   NULL;
    //char ByteArr64BitPakt[64];
    // int pktLen = objProto.mPktRegRead(0x10000000,&byArrPkt);
    // pktLen = EthPs01G->sendMessage(byArrPkt,pktLen,"127.0.0.1",12345);

    // QHostAddress ipAddress("127.0.0.1");
    // EthPs01G->readResponsPacket(ByteArr64BitPakt,pktLen,ipAddress,12345);
    // Proto objProtoRespPkt;
    // int ret	= objProtoRespPkt.mParseResponsePkt(ByteArr64BitPakt);
    // LOG_TO_FILE("RegVal 0x%X",ret);

    char *byArrPkt  =   NULL;
    char ByteArr64BitPakt[64];
    if(EthPs01G->getConStatus() == NOT_CONNECTED ){
        LOG_TO_FILE("Error: device not connected\n");
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Device not connected\n");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        LOG_TO_FILE("Return after showing msgbox\n");
        return;
    }

    int pktLen = objProto.mPktRegRead(0x10000004,&byArrPkt);
    if(pktLen == EthPs01G->sendMessage(byArrPkt,pktLen,EthPs01G->remote_ip,EthPs01G->remote_port)){
        QHostAddress ip(EthPs01G->remote_port);
        if(pktLen  == EthPs01G->readResponsPacket(ByteArr64BitPakt,pktLen,ip,port)){
            Proto objProtoRespPkt;
            int regval	= objProtoRespPkt.mParseResponsePkt(ByteArr64BitPakt);
             LOG_ONLY_DATA("Reg VAl: 0x%X",regval);
            if(regval > 0){
                ui->spinBox_device_setup_reg_val_read1->setValue(regval);
            }
            LOG_TO_FILE("Value updated in spinbox\n");
        }
    }else{
        LOG_TO_FILE("ERROR: Unable to send data to udp socket\n:");
    }
    delete byArrPkt;
    LOG_TO_FILE(":Exit==>");
}


