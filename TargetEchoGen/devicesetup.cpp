#include "devicesetup.h"
#include "ui_devicesetup.h"
#include <QMessageBox>
#include <QHostAddress>
#include "log.h"
#include "Proto.h"
#include "FileTransferAgent.h"


DeviceSetup::DeviceSetup(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DeviceSetup)
{
    ui->setupUi(this);

    EthPs01G = &UDP_PS1G_Con::getInstance();
    EthPl10G = &UDP_PL10G_Con::getInstance();
    EthPl01G = &UDP_PL1G_Con::getInstance();
    setupTransferAgent = new FileTransferAgent(this);

    QRegularExpression hexRegex("^(0x)?[0-9A-Fa-f]{1,8}$"); // max 8 hex digits
    QRegularExpressionValidator* hexValidator = new QRegularExpressionValidator(hexRegex, this);
    ui->lineEdit_reg_read_addr1->setValidator(hexValidator);
    ui->lineEdit_reg_read_addr2->setValidator(hexValidator);
    ui->lineEdit_reg_read_addr3->setValidator(hexValidator);
    ui->lineEdit_reg_read_addr4->setValidator(hexValidator);

    QIntValidator* validator = new QIntValidator(0, 2147483647);
    ui->lineEdit_mem_read_file_size->setValidator(validator);

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

    ui->lineEdit_devicesetup_mem_read_filename->setReadOnly(true);

    // // 🔄 Initialize QProgressDialog for percentage-based progress tracking
    // transferProgress = new QProgressDialog("Preparing file transfer...", "Cancel", 0, 100, this);
    // transferProgress->setWindowModality(Qt::WindowModal);
    // transferProgress->setWindowTitle("File Transfer Progress");
    // transferProgress->setAutoClose(false);     // Keep open until transfer finishes
    // transferProgress->setAutoReset(false);     // Manual control over reset
    // transferProgress->setMinimumDuration(1000); // Avoid premature popup (1s)
    // transferProgress->reset();                 // Clear stale values
    // transferProgress->hide();                  // Hide until file transfer starts

    // // Connect cancel behavior just once
    // connect(transferProgress, &QProgressDialog::canceled, this, [=]() {
    //     transferCanceled = true;
    //     setupTransferAgent->abortFileWrite(true);
    //     LOG_TO_FILE("User canceled the file transfer.");
    // });
    // connect(setupTransferAgent, &FileTransferAgent::progressUpdated,this, &DeviceSetup::updateTransferProgress);
    // connect(setupTransferAgent, &FileTransferAgent::close_progress_pop,this, &DeviceSetup::close_Progress_pop);

}

void DeviceSetup::close_Progress_pop(void){

    //transferProgress->reset();
    //transferProgress->close();
}
void DeviceSetup::updateTransferProgress(qint64 percentage){

    //transferProgress->setValue(qBound(0, percentage, 100));
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
            if (regval > 0) {
                lineEditVal->setText(QString("%1").arg(regval, 8, 16, QChar('0')).toUpper());
            }
        }
    } else {
        LOG_TO_FILE("ERROR: Unable to send data to udp socket.");
    }
    delete byArrPkt;
}

void DeviceSetup::WriteRegisterValue(QLineEdit* lineEditAddr, QLineEdit* lineEditVal) {

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

void DeviceSetup::on_pushButton_devsetup_mem_read_start_clicked()
{
    // if (!EthPs01G->getConStatus()) {
    //     Log::showStatusMessage(this, "Device not connected", "Device not connected");
    //     return;
    // }
    if(ui->radioButton_device_setup_ps_1g->isChecked()){

        QString filename = ui->lineEdit_devicesetup_mem_read_filename->text();
        int size  = ui->lineEdit_mem_read_file_size->text().toInt();
        if(size == 0 || ui->lineEdit_mem_read_file_size->text().isEmpty()){
            Log::showStatusMessage(this, "Device Setup", "Size is zero");
        }
        // setupTransferAgent->setupDevice(EthPs01G);
        // setupTransferAgent->configure(EthPs01G->remote_ip,EthPs01G->remote_port,filename,size,TargetToHost);
        // LOG_TO_FILE("PS 01G selected %d file name",size,filename.toStdString().c_str());
        // setupTransferAgent->start();
        // transferProgress->setRange(0,100);
        // transferProgress->show();
    }
}

void DeviceSetup::on_pushButton_device_setup_file_mem_read_clicked()
{
    QString title = "Device setup memory read";
    QString filePath = QFileDialog::getSaveFileName(this, title, QDir::homePath(), NULL);
    if (filePath.isEmpty()) {
        LOG_TO_FILE("User canceled file creation dialog.");
        return;
    }
    LOG_TO_FILE("Creating file: %s", filePath.toUtf8().constData());
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream out(&file);
        out << "Created by Log::showFileCreateDialog\n";
        file.close();
        ui->lineEdit_devicesetup_mem_read_filename->setText(filePath);
        LOG_TO_FILE("File successfully written.");
    } else {
        LOG_TO_FILE("Error creating file: %s", file.errorString().toUtf8().constData());
    }
}

