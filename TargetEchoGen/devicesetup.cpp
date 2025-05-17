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
    ret	= objProtoRespPkt.mPktParse(byArrPktResp);
    LOG_TO_FILE("objProtoRespPkt.mPktParse ret val:%d\n",ret);
    ret     *=  10;
    if(ret < 0){
        LOG_TO_FILE("Failed to parse packet\n");
    }else{
        LOG_TO_FILE("Packet parsing done:\n");
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
    int pktLen = objProto.mPktRegRead(0x12345678,&byArrPkt);
    if(pktLen == EthPs01G->sendMessage(byArrPkt,pktLen,"127.0.0.1",12345)){

        if(pktLen  == EthPs01G->readResponsPacket(ByteArr64BitPakt,pktLen,ipAddress,port)){

            uint64_t ret = ParseRegReadResponsePkt(ByteArr64BitPakt,pktLen);
            if(ret > 0){
                ui->spinBox_device_setup_reg_val_read1->setValue(ret);
            }
            LOG_TO_FILE("Value updated in spinbox\n");
        }
    }else{
        LOG_TO_FILE("ERROR: Unable to send data to udp socket\n:");
    }
    delete byArrPkt;
    LOG_TO_FILE(":Exit:\n");
}


