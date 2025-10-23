#include "mainwindow.h"
#include "devicesetup.h"
#include "fileprocessing.h"
#include "filesender.h"
#include "rf.h"
#include "selftest.h"
#include "spectrum.h"
#include "ui_mainwindow.h"
#include "dachelper.h"
#include <QTableWidgetItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDir>
#include <QFileInfoList>
#include <QHostAddress>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    conn = new ConnectionType(this);
    DeviceSetup *deviceSetup = new DeviceSetup(this);

    ui->tabWidgetMainTab->addTab(new FileProcessing(),"File processing");
    ui->tabWidgetMainTab->addTab(deviceSetup,"Device setup");
    ui->tabWidgetMainTab->addTab(new SelfTest(),"Self Test");
    ui->tabWidgetMainTab->addTab(new Spectrum(),"Spectrum Analyzer");
    ui->tabWidget->addTab(new RF(),"RF");

    ui->PbRefresh->setIconSize(QSize(ui->PbRefresh->width(), ui->PbRefresh->height()));
    ui->PbConnSettings->setIconSize(QSize(ui->PbConnSettings->width(), ui->PbConnSettings->height()));
    ui->PbConnReset->setIconSize(QSize(ui->PbConnReset->width(), ui->PbConnReset->height()));
    ui->label_device_temp_dig_val->setText(tr("%1 °C").arg(100));
    ui->label_device_temp_ana_val->setText(tr("%1 °C").arg(100));
    load_files();


    ui->SbDAC1AMPLFixedLevel->setRange(-27,0);
    ui->SbDAC2AMPLFixedLevel->setRange(-27,0);
    ui->SbDAC1AMPLBaseValue->setRange(-27,0);
    ui->SbDAC2AMPLBaseValue->setRange(-27,0);

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
    // connect(setupTransferAgent, &FileTransferAgent::progressUpdated,this, &MainWindow::updateTransferProgress);
    // connect(setupTransferAgent, &FileTransferAgent::close_progress_pop,this, &MainWindow::close_Progress_pop);

    connect(conn, &ConnectionType::connectionSucceeded, this, &MainWindow::onConnectionSuccess);
    connect(conn, &ConnectionType::connectionFailed, this, &MainWindow::onConnectionFailure);

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

MainWindow::~MainWindow()
{
    delete file_processing;
    delete device_setup;
    delete ui;
}

void MainWindow::onConnectionSuccess(iface eInterface)
{
    switch(eInterface){
    case eNONE:
        break;
    case eETHPS1G:
        ui->LblConnPS1GStatusLed->setPixmap(QPixmap(":/images/led-green_icon.jpg"));
        break;
    case eETHPL1G:
        ui->LblConnPL1GStatusLed->setPixmap(QPixmap(":/images/led-green_icon.jpg"));
        break;
    case eETH10G:
        ui->LblConnPL10GStatusLed->setPixmap(QPixmap(":/images/led-green_icon.jpg"));
        break;
    case eSERIAL:
        break;
    case ePCIe:
        break;
    default:
        LOG_INFO("Ivalide interface\n");
    }
    // Handle success (e.g., update UI, enable features)
}

void MainWindow::onConnectionFailure(iface eInterface)
{
    switch(eInterface){
    case eNONE:
        break;
    case eETHPS1G:
        ui->LblConnPS1GStatusLed->setPixmap(QPixmap(":/images/led-icon-red.jpg"));
        break;
    case eETHPL1G:
        ui->LblConnPL1GStatusLed->setPixmap(QPixmap(":/images/led-icon-red.jpg"));
        break;
    case eETH10G:
        ui->LblConnPL10GStatusLed->setPixmap(QPixmap(":/images/led-icon-red.jpg"));
        break;
    case eSERIAL:
        break;
    case ePCIe:
        break;
    default:
        LOG_INFO("Ivalide interface\n");
    }
}
void MainWindow::onTimeout()
{

}

void MainWindow::load_files()
{
        QDir directory("C:\\Users\\razzz\\OneDrive\\Documents\\TargetEchoGen\\data");
        QStringList files = directory.entryList(QDir::Files);

        for (int row = 0; row < files.size(); ++row) {
            QFileInfo fileInfo(directory, files.at(row));
            QString modifiedDate = fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss");
            double fileSizeMB = fileInfo.size() / (1024.0 * 1024.0); // Convert size to MB

            QTableWidgetItem *fileNameItem = new QTableWidgetItem(files.at(row));
            QTableWidgetItem *modifiedDateItem = new QTableWidgetItem(modifiedDate);
            QTableWidgetItem *fileSizeItem = new QTableWidgetItem(QString::number(fileSizeMB, 'f', 2));

            // Make items read-only
            fileNameItem->setFlags(fileNameItem->flags() & ~Qt::ItemIsEditable);
            modifiedDateItem->setFlags(modifiedDateItem->flags() & ~Qt::ItemIsEditable);
            fileSizeItem->setFlags(fileSizeItem->flags() & ~Qt::ItemIsEditable);

        }
}

void MainWindow::close_Progress_pop(void){

    //transferProgress->reset();
   // transferProgress->close();
}
void MainWindow::updateTransferProgress(qint64 percentage){

    //transferProgress->setValue(qBound(0, percentage, 100));
}

// void MainWindow::on_pb_ddr_dac_iq_file_browse_clicked()
// {
//     LOG_TO_FILE(":Entry==>");
//     QString fileName = QFileDialog::getOpenFileName(this,
//                                                     "Open File",
//                                                     ".",
//                                                     "Text Files (*.bin);;All Files (*)");
//     if (!fileName.isEmpty())
//     {
//         qDebug() << "FileName" << fileName;
//         LOG_TO_FILE("Filename:%s",fileName.toStdString().c_str());
//         //ui->lineEdit_ddr_dac_iq_file_name_path->setText(fileName);
//     }
//     LOG_TO_FILE(":Exit==>\n");
// }

void MainWindow::FileReadWriteSetup(iface deviceType, uint iFileSize, QString sFilePath, eXferDir dir)
{

    LOG_INFO("MainWindow::FileReadWriteSetup() <ENTER>");
    char* byArrPkt = nullptr;
    Proto protocolobj;
    switch (deviceType)
    {
    case iface::eSERIAL:
    {
        serial = UartSerial::getInstance();
        if (!serial)
        {
            LOG_TO_FILE("ERROR: Serial pointer is null.");
            return;
        }
        break;
    }
    case iface::eETHPL1G:
    {
        stFileReadWriteConf Cnf;
        Cnf.iFileSize = iFileSize;
        Cnf.sFilePath = sFilePath;
        Cnf.eInterface = iface::eETH10G;
        Cnf._Dir = dir;
        setupTransferAgent->configure(Cnf);

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
    LOG_INFO("MainWindow::FileReadWriteSetup() <EXIT>");

}

void MainWindow::on_PbConnSettings_clicked()
{
    conn->show();
}


void MainWindow::on_PbDAC1IQFileSend_clicked()
{

}

void MainWindow::on_PbDAC2TgrSetup_clicked()
{
    uint32_t trigSourceSelect = 0;
    uint32_t pwSamples = 0;
    uint32_t signalDelay= 0;
    uint32_t pulseGap = 0;
    uint32_t pulseWidth= 0;
    uint32_t triggerEnable=1;

    pulseWidth = ui->LeDAC2TgrPulseWidthSample->text().toUInt();
    pulseGap = ui->LeDAC2TgrPulseGapSample->text().toUInt();

    DacHelper::LxTriggerSetup(eETHPL1G,trigSourceSelect,pwSamples,signalDelay,pulseGap,pulseWidth,triggerEnable);
}


void MainWindow::on_PbDAC2TgrStart_clicked()
{
    DacHelper::LxTriggerStart(eETHPL1G);
}


void MainWindow::on_PbDAC2TgrStop_clicked()
{
   DacHelper::LxTriggerStop(eETHPL1G);
}


void MainWindow::on_PbDAC1AMPLFixedLevelSet_clicked()
{
    if(ui->RbDAC1AMPLFixedLevel->isChecked())
    {
        uint32_t dbm = ui->SbDAC1AMPLFixedLevel->value();
        DacHelper::FixAmplSetting(eETHPL1G,dbm);
    }
}

void MainWindow::on_ChkBoxDAC1NOCEnable_checkStateChanged(const Qt::CheckState &arg1)
{
    if(arg1 == Qt::Checked){
        DacHelper::Enable_nco(eETHPL1G);
    }else{
        DacHelper::Disable_nco(eETHPL1G);
    }
}


void MainWindow::on_PbDAC1TriggerSetup_clicked()
{
    uint32_t trigSourceSelect = 0;
    uint32_t pwSamples = 0;
    uint32_t signalDelay= 0;
    uint32_t pulseGap = 0;
    uint32_t pulseWidth= 0;
    uint32_t triggerEnable=1;

    pulseWidth = ui->LeDAC1TgrPulseWidthSample->text().toUInt();
    pulseGap = ui->LeDAC1TgrPulseGapSample->text().toUInt();

    DacHelper::LxTriggerSetup(eETHPL1G,trigSourceSelect,pwSamples,signalDelay,pulseGap,pulseWidth,triggerEnable);
}

void MainWindow::on_PBdac1TSstart_clicked()
{
    DacHelper::LxTriggerStart(eETHPL1G);
}

void MainWindow::on_PBdac1TSstop_clicked()
{
    DacHelper::LxTriggerStop(eETHPL1G);
}

void MainWindow::on_PbDAC2AMPLFixedLevelSet_clicked()
{

}


void MainWindow::on_PbDAC1AMPLBaseValueIncr_clicked()
{
    if(ui->RbDAC1AMPLBaseValue->isChecked())
    {
        int dbmval = ui->SbDAC1AMPLBaseValue->value();
        dbmval += 1;
        ui->SbDAC1AMPLBaseValue->setValue(dbmval);
        //DacHelper::FixAmplSetting(eETHPL1G,dbmval);
    }
}


void MainWindow::on_PbDAC1AMPLBaseValueDecr_clicked()
{
    if(ui->RbDAC1AMPLBaseValue->isChecked())
    {
        int dbmval = ui->SbDAC1AMPLBaseValue->value();
        dbmval -= 1;
        ui->SbDAC1AMPLBaseValue->setValue(dbmval);
        DacHelper::FixAmplSetting(eETHPL1G,dbmval);
    }
}


void MainWindow::on_PbDAC2AMPLBaseValueIncr_clicked()
{
    if(ui->RbDAC2AMPLBaseValue->isChecked())
    {
        int dbmval = ui->SbDAC2AMPLBaseValue->value();
        dbmval += 1;
        ui->SbDAC2AMPLBaseValue->setValue(dbmval);
        DacHelper::FixAmplSetting(eETHPL1G,dbmval);
    }
}


void MainWindow::on_PbDAC2AMPLBaseValueDncr_clicked()
{
    if(ui->RbDAC2AMPLBaseValue->isChecked())
    {
        int dbmval = ui->SbDAC2AMPLBaseValue->value();
        dbmval -= 1;
        ui->SbDAC2AMPLBaseValue->setValue(dbmval);
        DacHelper::FixAmplSetting(eETHPL1G,dbmval);
    }
}


void MainWindow::on_PbDAC1Apply_clicked()
{
    int iPwSample = ui->SbDAC1PWSamples->value();
    int iSignalDelay  = ui->SbDAC1SignalDelay->value();
    int iDoplerShiftHz = ui->SbDAC1DopplerShiftHz->value();

    DacHelper::WrIterpolation(eETHPL1G, ui->CbIDAC1InterpSelect->currentText().toInt());
    DacHelper::WrNCOFrq(eETHPL1G,ui->CbDAC1NOCFrequency->currentText());
    Utils::RegisterWrite(eETHPL1G,AVR_DAC3_BASE_ADDR+0x08,iPwSample);
    Utils::RegisterWrite(eETHPL1G,AVR_DAC3_BASE_ADDR+0x6C,iSignalDelay);
    Utils::RegisterWrite(eETHPL1G,AVR_DAC3_BASE_ADDR+0x40,iDoplerShiftHz);

}


void MainWindow::on_PbDAC1SUMRefresh_clicked()
{

    uint32_t val   = Utils::RegRead(eETHPL1G,AVR_DAC3_BASE_ADDR+0x74);
    ui->LbIDAC1SUMNumOfTriggersVal->setText(QString::number(val));
}


void MainWindow::on_ChkBoxNBADCDDSEnable_checkStateChanged(const Qt::CheckState &arg1)
{
    if(arg1 == Qt::Checked)
    {
        Utils::RegisterWrite(eETHPL1G,0x508,1);
    }
    else{
        Utils::RegisterWrite(eETHPL1G,0x508,0);
    }
}

uint64_t MainWindow::ComputeDDSFCW(uint32_t fcw, uint32_t fs)
{
    // Log input values
    LOG_INFO("ComputeDDSFCW <ENTER> fcw: %u, fs: %u", fcw, fs);

    if (fs == 0) {
        LOG_ERROR("Sampling frequency cannot be zero");
        return 0;
    }

    // Compute ratio with floating-point precision
    double ratio = static_cast<double>(fcw) / static_cast<double>(fs);
    double scaled = ratio * 4294967296.0;  // 2^32

    // Log intermediate values
    LOG_INFO("Ratio: %.10f, Scaled: %.2f", ratio, scaled);

    uint64_t tuningWord = static_cast<uint64_t>(scaled);

    // Log final result
    LOG_INFO("ComputeDDSFCW <EXIT> Tuning Word: %llu", tuningWord);
    return tuningWord;
}


void MainWindow::on_PbNB_ADC_DDSFCWSet_clicked()
{
    uint fcw = ui->LineEditNBADCFcwVal->text().toUInt();
    uint fs = ui->lineEditNBADCDDSFs_val->text().toUInt();
    //ui->lineEditWBCICInputFs->setText(ui->lineEditWBDDSFs_val->text());

    uint64_t totalval = ComputeDDSFCW(fcw,fs);
    Utils::RegisterWrite(eETHPL1G,0x504,totalval);
    Utils::RegisterWrite(eETHPL1G,0x508,3);
    Utils::RegisterWrite(eETHPL1G,0x508,1);
}
