#include "devicesetup.h"
#include "ui_devicesetup.h"
#include <QMessageBox>
#include <QHostAddress>
#include "log.h"
#include "Proto.h"
#include "FileTransferAgent.h"
#include "Utils.h"

DeviceSetup::DeviceSetup(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DeviceSetup)
{
    ui->setupUi(this);

    QRegularExpression hexRegex("^(0x)?[0-9A-Fa-f]{1,8}$"); // max 8 hex digits
    QRegularExpressionValidator* hexValidator = new QRegularExpressionValidator(hexRegex, this);
    ui->LeRegReadAddr1->setValidator(hexValidator);
    ui->LeRegReadAddr2->setValidator(hexValidator);
    ui->LeRegReadAddr3->setValidator(hexValidator);
    ui->LeRegReadAddr4->setValidator(hexValidator);

    QIntValidator* validator = new QIntValidator(0, 2147483647);
    ui->LeMemReadFileNameReadSize->setValidator(validator);

    ui->LeRegReadVal1->setReadOnly(true);
    ui->LeRegReadVal2->setReadOnly(true);
    ui->LeRegReadVal3->setReadOnly(true);
    ui->LeRegReadVal4->setReadOnly(true);

    ui->LeRegWriteAddr1->setValidator(hexValidator);
    ui->LeRegWriteAddr2->setValidator(hexValidator);
    ui->LeRegWriteAddr3->setValidator(hexValidator);
    ui->LeRegWriteAddr4->setValidator(hexValidator);


    ui->LeRegWriteVal1->setValidator(hexValidator);
    ui->LeRegWriteVal2->setValidator(hexValidator);
    ui->LeRegWriteVal3->setValidator(hexValidator);
    ui->LeRegWriteVal4->setValidator(hexValidator);

    //ui->lineEdit_devicesetup_mem_read_filename->setReadOnly(true);

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

    setupTransferAgent = new FileTransferAgent();
    progressDialog = new TransferProgressDialog(this); // Pass your QWidget parent
    // Connect progress signal
    connect(setupTransferAgent,&FileTransferAgent::progressUpdated,progressDialog, &TransferProgressDialog::updateProgress);

    // Connect cancel signal
    connect(progressDialog, &TransferProgressDialog::cancelRequested,setupTransferAgent, &FileTransferAgent::abortTransfer);

    // Optional: Close dialog when transfer completes
    connect(setupTransferAgent, &FileTransferAgent::transferComplete,progressDialog, &QDialog::accept);
    progressDialog->hide();


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
void DeviceSetup::FileReadWriteSetup(iface deviceType, uint iFileSize, QString sFilePath, eXferDir dir)
{

    LOG_INFO("DeviceSetup::FileReadWriteSetup()<ENTER>");
    char* byArrPkt = nullptr;
    Proto protocolobj;
    switch (deviceType)
    {
    case iface::eSERIAL:
    {
        stFileReadWriteConf Cnf;
        Cnf.iFileSize = iFileSize;
        Cnf.sFilePath = sFilePath;
        Cnf.eInterface = iface::eSERIAL;
        Cnf._Dir = dir;
        LOG_INFO("eSERIAL: iFileSize:%d,sFilePath:%s",iFileSize,sFilePath.toStdString().c_str());
        setupTransferAgent->configure(Cnf);
        setupTransferAgent->start();
        progressDialog->show();
        break;
    }
    case iface::eETHPL1G:
    {
        stFileReadWriteConf Cnf;
        Cnf.iFileSize = iFileSize;
        Cnf.sFilePath = sFilePath;
        Cnf.eInterface = iface::eETHPL1G;
        Cnf._Dir = dir;
        LOG_INFO("eETHPL1G: iFileSize:%d,sFilePath:%s",iFileSize,sFilePath.toStdString().c_str());
        setupTransferAgent->configure(Cnf);
        setupTransferAgent->start();
        progressDialog->show();

    }
    break;
    case iface::ePCIe:
        break;
    case iface::eETH10G:
    {
        stFileReadWriteConf Cnf;
        Cnf.iFileSize = iFileSize;
        Cnf.sFilePath = sFilePath;
        Cnf.eInterface = iface::eETH10G;
        Cnf._Dir = dir;
        LOG_INFO("eETH10G: iFileSize:%d,sFilePath:%s",iFileSize,sFilePath.toStdString().c_str());
        setupTransferAgent->configure(Cnf);
        setupTransferAgent->start();
        progressDialog->show();
    }
    break;
    default:
        LOG_INFO("No valid interface selection");
    }
    delete byArrPkt;
    LOG_INFO("DeviceSetup::FileReadWriteSetup()<EXIT>");
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


// void DeviceSetup::readRegisterValue(QLineEdit* lineEditAddr, QLineEdit* lineEditVal) {

//     char* byArrPkt = nullptr;
//     char ByteArr64BitPakt[64];

//     if (!EthPs01G->getConStatus()) {
//         Log::showStatusMessage(this, "Device not connected", "Device not connected");
//         return;
//     }
//     bool ok;
//     uint addr = lineEditAddr->text().toUInt(&ok, 16);
//     if (!ok) {
//         LOG_TO_FILE("ERROR: Invalid address format.");
//         return;
//     }

//     int pktLen = objProto.mPktRegRead(addr, &byArrPkt);
//     if (pktLen == EthPs01G->sendMessage(byArrPkt, pktLen, EthPs01G->remote_ip, EthPs01G->remote_port)) {
//         QHostAddress ip(EthPs01G->remote_ip);
//         if (pktLen == EthPs01G->readResponsPacket(ByteArr64BitPakt, pktLen, ip, port)) {
//             Proto objProtoRespPkt;
//             int regval = objProtoRespPkt.mParseResponsePkt(ByteArr64BitPakt);
//             if (regval > 0) {
//                 lineEditVal->setText(QString("%1").arg(regval, 8, 16, QChar('0')).toUpper());
//             }
//         }
//     } else {
//         LOG_TO_FILE("ERROR: Unable to send data to udp socket.");
//     }
//     delete byArrPkt;
// }

// void DeviceSetup::WriteRegisterValue(QLineEdit* lineEditAddr, QLineEdit* lineEditVal) {

//     char* byArrPkt = nullptr;
//     char ByteArr64BitPakt[64];
//     if (!EthPs01G->getConStatus()) {
//         Log::showStatusMessage(this, "Device not connected", "Device not connected");
//         return;
//     }
//     bool ok;
//     uint addr = lineEditAddr->text().toUInt(&ok, 16);
//     if (!ok) {
//         LOG_TO_FILE("ERROR: Invalid address format.");
//         return;
//     }
//     uint val = lineEditVal->text().toUInt(&ok, 16);
//     if (!ok) {
//         LOG_TO_FILE("ERROR: Invalid address format.");
//         return;
//     }
//     int pktLen = objProto.mPktRegWrite(addr,val,&byArrPkt);
//     if (pktLen == EthPs01G->sendMessage(byArrPkt, pktLen, EthPs01G->remote_ip, EthPs01G->remote_port)){

//         QHostAddress ip(EthPs01G->remote_ip);
//         //Read and flushed fd
//         if (pktLen == EthPs01G->readResponsPacket(ByteArr64BitPakt, pktLen, ip, port)) {
//         }
//     } else {
//         LOG_TO_FILE("ERROR: Unable to send data to udp socket.");
//     }
//     delete byArrPkt;
// }
void DeviceSetup::on_PbRegRead1_clicked()
{
    Utils::readRegisterValue(eETHPL1G,ui->LeRegReadAddr1,ui->LeRegReadVal1);
}


void DeviceSetup::on_PbRegWrite1_clicked()
{
    bool ok;
    uint iAddr = ui->LeRegWriteAddr1->text().toUInt(&ok,16);
    uint iVal = ui->LeRegWriteVal1->text().toUInt(&ok,16);
    Utils::RegisterWrite(eETHPL1G,iAddr,iVal);
}

void DeviceSetup::on_PbMemReadFileNameBrowse_clicked()
{
        LOG_INFO("DeviceSetup::on_PbMemReadFileNameBrowse_clicked() <ENTER>");
        QString fileName = QFileDialog::getSaveFileName(this,"Open File",".","Text Files (*.bin);;All Files (*)");
        if (!fileName.isEmpty())
        {
            LOG_INFO("Filename:%s",fileName.toStdString().c_str());
            ui->LeMemReadFileNamePath->setText(fileName);
        }
        LOG_INFO("DeviceSetup::on_PbMemReadFileNameBrowse_clicked() <EXIT>\n");
}

void DeviceSetup::on_PbMemReadRead_clicked()
{
    FileReadWriteSetup(eETH10G,ui->LeMemReadFileNameReadSize->text().toInt(),ui->LeMemReadFileNamePath->text(),eRead);
}


void DeviceSetup::on_PbMemWriteFileBrowse_clicked()
{
    QString FileName = QFileDialog::getOpenFileName(this,"Open File",".","Text Files (*.bin);;All Files (*)");
    if (!FileName.isEmpty())
    {
        LOG_INFO("Filename:%s",FileName.toStdString().c_str());
        ui->LeMemWriteFileNamePath->setText(FileName);
    }
}

void DeviceSetup::on_PbMemWrite_clicked()
{
    FileReadWriteSetup(eETH10G,ui->LeMemWriteFileSize->text().toInt(),ui->LeMemWriteFileNamePath->text(),eWrite);
}

