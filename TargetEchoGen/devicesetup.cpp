#include "devicesetup.h"
#include "ui_devicesetup.h"
#include <QMessageBox>
#include <QHostAddress>
#include "log.h"
#include "proto.h"
#include "FileTransferAgent.h"
#include "Utils.h"
#include "devicesetuphelper.h"

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

    setupTransferAgent = new FileTransferAgent();
    progressDialog = new TransferProgressDialog(this); // Pass your QWidget parent

    //Progress bar signal connection
    connect(setupTransferAgent,&FileTransferAgent::progressUpdated,progressDialog, &TransferProgressDialog::updateProgress);
    connect(progressDialog, &TransferProgressDialog::cancelRequested,setupTransferAgent, &FileTransferAgent::abortTransfer);
    connect(setupTransferAgent, &FileTransferAgent::transferComplete,progressDialog, &QDialog::accept);
    progressDialog->hide();


}

DeviceSetup::~DeviceSetup()
{
    delete ui;
}

iface DeviceSetup::getSelectedDeviceType()
{
    if (ui->RbPS1G->isChecked())       return eETHPS1G;
    if (ui->RbPL1G->isChecked())       return eETHPL1G;
    if (ui->RbPL10G->isChecked())      return eETH10G;
    if (ui->RbPSSerial->isChecked())   return eSERIAL;
    if (ui->RbPLSerial->isChecked())   return ePLSERIAL;

    Log::showStatusMessage(this, "Device Setup", "Please select an interface");
    return eNONE;
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


void DeviceSetup::on_PbLMKInitDefault_clicked()
{
    LOG_INFO("DeviceSetup::on_PbLMKInitDefault_clicked() <ENTER>\n");
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE){
        LOG_ERROR("Interface not selected %d",deviceType);
        return;
    }

    const QString freq = ui->CbLMKInit->currentText().trimmed();
    if (freq == QLatin1String("60MHz")) {
        DeviceSetupHelper::LmkDefault60MhzSetting(deviceType);
    } else if (freq == QLatin1String("120MHz")) {
        DeviceSetupHelper::LmkDefault120MhzSetting(deviceType);
    } else {

    }
    LOG_INFO("DeviceSetup::on_PbLMKInitDefault_clicked() <EXIT>\n");
}


void DeviceSetup::on_PbDACInitDefault_clicked()
{
    LOG_INFO("DeviceSetup::on_PbDACInitDefault_clicked() <ENTER>\n");
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE){
        LOG_ERROR("Interface not selected %d",deviceType);
        return;
    }

    const QString dac = ui->CbDACInit->currentText().trimmed();
    if (dac == QLatin1String("DAC 1")) {
    }
    else if (dac == QLatin1String("DAC 2")) {

    }
    else if (dac == QLatin1String("DAC 3")) {
        DeviceSetupHelper::Dac2DefaultSetting(deviceType);
    }
    else {}
    LOG_INFO("DeviceSetup::on_PbDACInitDefault_clicked() <EXIT>\n");
}

