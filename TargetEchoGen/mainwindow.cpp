#include <QTableWidgetItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDir>
#include <QFileInfoList>
#include <QHostAddress>
#include <QFileDialog>
#include <QMessageBox>
#include "mainwindow.h"
#include "devicesetup.h"
#include "fileprocessing.h"
#include "filesender.h"
#include "rf.h"
#include "selftest.h"
#include "spectrum.h"
#include "ui_mainwindow.h"
#include "dachelper.h"
#include "connectionctx.h"
#include <QMetaType>
#include "packetforwarder.h"   // for RelayMeasurement

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qRegisterMetaType<RelayMeasurement>("RelayMeasurement");
    ui->scrollArea->setWidgetResizable(false);
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->scrollArea->setMinimumSize(QSize(0,0));
    ui->scrollArea->setFrameShape(QFrame::NoFrame);
    //Adjust the value to when that scroll bar appear
    ui->scrollAreaWidgetContents->setFixedSize(1925,1025);  // or setMinimumSize(1920,1080)
    ui->scrollAreaWidgetContents->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    this->setMinimumSize(800,600);

    conn = new ConnectionType(this);

    deviceSetup   = new DeviceSetup(this);
    fileProcessing= new FileProcessing(this);
    selfTest      = new SelfTest(this);
    spectrum      = new Spectrum(this);
    rf            = new RF(this);


    ui->tabWidgetMainTab->addTab(fileProcessing, "File processing");
    ui->tabWidgetMainTab->addTab(deviceSetup,   "Device setup");
    ui->tabWidgetMainTab->addTab(selfTest,      "Self Test");
    ui->tabWidgetMainTab->addTab(spectrum,      "Spectrum Analyzer");
    ui->tabWidget->addTab(rf,                   "RF");

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

    ui->LeTopDateTime->setText("Date & Time:  "+ QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    setupTransferAgent = new FileTransferAgent();
    progressDialog = new TransferProgressDialog(this); // Pass your QWidget parent
    // Connect progress signal
    connect(setupTransferAgent,&FileTransferAgent::progressUpdated,progressDialog, &TransferProgressDialog::updateProgress);

    // Connect cancel signal
    connect(progressDialog, &TransferProgressDialog::cancelRequested,setupTransferAgent, &FileTransferAgent::abortTransfer);

    // Optional: Close dialog when transfer completes
    connect(setupTransferAgent, &FileTransferAgent::transferComplete,progressDialog, &QDialog::accept);
    progressDialog->hide();

    // Optional: Close dialog when transfer completes
    connect(setupTransferAgent, &FileTransferAgent::transferComplete,this, &MainWindow::TransferDone);

    //Connection and selection handling
    auto& helper = ConnectionHelper::instance();
    connect(&helper, &ConnectionHelper::stateChanged,  this, &MainWindow::onConnStateChanged);
    connect(&helper, &ConnectionHelper::activeChanged, this, [this](iface active){
        onConnStateChanged(active, ConnectionHelper::instance().info(active));
    });
    connect(&helper, &ConnectionHelper::selectedChanged, this, [this](iface sel){
        onConnStateChanged(sel, ConnectionHelper::instance().info(sel));
    });

    // Example: radio button for PL1G selected in MainWindow
    connect(ui->RbPL1GSel, &QRadioButton::toggled, this, [](bool on){
        if (on) ConnectionHelper::instance().setSelected(eETHPL1G);
    });

    connect(ui->RbPS1GSel, &QRadioButton::toggled, this, [](bool on){
        if (on) ConnectionHelper::instance().setSelected(eETHPS1G);
    });

    connect(ui->RbPL10GSel, &QRadioButton::toggled, this, [](bool on){
        if (on) ConnectionHelper::instance().setSelected(eETH10G);
    });

    connect(ui->RbPSSerialSel, &QRadioButton::toggled, this, [](bool on){
        if (on) ConnectionHelper::instance().setSelected(eSERIAL);
    });

    connect(ui->RbPLSerialSel, &QRadioButton::toggled, this, [](bool on){
        if (on) ConnectionHelper::instance().setSelected(eSERIAL);
    });

    connect(conn, &ConnectionType::connectionSucceeded, this, &MainWindow::onConnectionSuccess);
    connect(conn, &ConnectionType::connectionFailed, this, &MainWindow::onConnectionFailure);

}

MainWindow::~MainWindow()
{
    delete file_processing;
    delete device_setup;
    delete ui;
}

void MainWindow::TransferDone(eStatus DoneStatus){

    switch (DoneStatus) {
    case eReadDone:
        break;
    case eWriteDone:
        SetDAC1ReadSettingAfterFileSend();
        break;
    default:
        break;
    }
}

void MainWindow::onConnStateChanged(iface which, ConnInfo s)
{
    QLabel* ledLabel = nullptr;
    QLabel* StatusTextlbl = nullptr;
    DeviceStatus state;
    // Select which LED to update
    switch (which)
    {
    case eETHPS1G:
        ledLabel = ui->LblConnPS1GStatusLed;
        StatusTextlbl = ui->LblConStatusPS01G;
        break;
    case eETHPL1G:
        ledLabel = ui->LblConnPL1GStatusLed;
        StatusTextlbl = ui->LblConStatusPL01G;
        break;
    case eETH10G:
        ledLabel = ui->LblConnPL10GStatusLed;
        StatusTextlbl = ui->LblConStatusPL10G;
        break;
    default:
        return; // ignore unsupported ones
    }

    if (!ledLabel)
        return;

    // Check if it's the currently active (or selected) connection
    auto& ctx = ConnectionHelper::instance();
    const bool isActive   = (ctx.activeInterface() == which);
    const bool isSelected = (ctx.selectedInterface() == which);

    if (!s.connected) {
        state = DeviceStatus::Disconnected;
    }
    else if (isSelected) {
        state = DeviceStatus::Selected;
    }
    else if (isActive) {
        state = DeviceStatus::Active;
    }
    else {
        state = DeviceStatus::Idle;
    }

    // Now apply UI
    ledLabel->setPixmap(QPixmap(Utils::statusToIcon(state)));
    StatusTextlbl->setText(Utils::statusToText(state));

}

void MainWindow::onConnectionSuccess(iface eInterface)
{
    switch(eInterface){
    case eNONE:
        break;
    case eETHPS1G:
        ui->LblConnPS1GStatusLed->setPixmap(QPixmap(":/images/green-checked-radio-button-48.png"));
        break;
    case eETHPL1G:
        ui->LblConnPL1GStatusLed->setPixmap(QPixmap(":/images/green-checked-radio-button-48.png"));
        break;
    case eETH10G:
        ui->LblConnPL10GStatusLed->setPixmap(QPixmap(":/images/green-checked-radio-button-48.png"));
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
        //ui->LblConnPS1GStatusLed->setPixmap(QPixmap(":/images/led-icon-red.jpg"));
        ui->LblConnPS1GStatusLed->setPixmap(QPixmap(":/images/icons8-red-notconn-cross-48.png"));
        break;
    case eETHPL1G:
        //ui->LblConnPL1GStatusLed->setPixmap(QPixmap(":/images/led-icon-red.jpg"));
        ui->LblConnPL1GStatusLed->setPixmap(QPixmap(":/images/icons8-red-notconn-cross-48.png"));
        break;
    case eETH10G:
        //ui->LblConnPL10GStatusLed->setPixmap(QPixmap(":/images/led-icon-red.jpg"));
        ui->LblConnPL10GStatusLed->setPixmap(QPixmap(":/images/icons8-red-notconn-cross-48.png"));
        break;
    case eSERIAL:
        break;
    case ePCIe:
        break;
    default:
        LOG_INFO("Ivalide interface\n");
    }
}

iface MainWindow::getSelectedDeviceType()
{
    auto& ctx = ConnectionHelper::instance();
    iface sel = ctx.selectedInterface();

    const QString ifaceName = Utils::ifaceToQString(sel);

    // 1️⃣ Check if no interface selected
    if (sel == eNONE)
    {
        LOG_ERROR("[MainWindow] No interface selected (iface=%s)", Utils::ifaceToCStr(sel));
        Log::showStatusMessage(this, "Device Setup", "Please select an interface before proceeding.");
        return eNONE;
    }

    // 2️⃣ Check if selected interface is connected
    ConnInfo info = ctx.info(sel);
    if (!info.connected)
    {
        LOG_ERROR("[MainWindow] Selected interface '%s' is NOT connected.", Utils::ifaceToCStr(sel));
        Log::showStatusMessage(this, "Device Setup",
                               QString("Selected interface '%1' is not connected.").arg(ifaceName));
        return eNONE;
    }

    // 3️⃣ Success — valid and connected interface
    LOG_INFO("[MainWindow] Selected and connected interface: %s", Utils::ifaceToCStr(sel));
    //Log::showStatusMessage(this, "Device Setup", QString("Selected Interface: %1").arg(ifaceName));

    return sel;
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

void MainWindow::FileReadWriteSetup(iface deviceType, qint64 iFileSize, QString sFilePath, eXferDir dir)
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
            LOG_ERROR("Serial pointer is null.");
            return;
        }
        break;
    }
    case iface::eETHPL1G:
    {
        stFileReadWriteConf Cnf;
        Cnf.iFileSize = iFileSize;
        Cnf.sFilePath = sFilePath;
        Cnf.eInterface = deviceType;
        Cnf._Dir = dir;
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
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("[WriteRegister] Interface not selected");
        return;
    }
    const QString filename = ui->LeDAC1IQFileName->text();
    QFileInfo fileInfo(filename);

    if (!fileInfo.exists() || !fileInfo.isFile()) {
        LOG_ERROR("[WriteRegister] File does not exist: %s", qPrintable(filename));
        return;
    }

    // File size in bytes (up to 64-bit)
    quint64 fileSize = static_cast<quint64>(fileInfo.size());
    if (fileSize % 4096 != 0) {
        fileSize = ((fileSize + 4095) / 4096) * 4096;
    }
    //Start address
    Utils::RegWrite(deviceType,0x100, 0x00);
    Utils::RegWrite(deviceType, 0x104,0x00);

    quint64 WriteFileSize = fileSize;

    // Split into two 32-bit parts
    write_size_lo = static_cast<quint32>(WriteFileSize & 0xFFFFFFFFULL);
    write_size_hi = static_cast<quint32>((WriteFileSize >> 32) & 0xFFFFFFFFULL);

    // Write lower 32 bits to 0x100
    Utils::RegWrite(deviceType,0x108, write_size_lo);
    //Write upper 32 bits to 0x128 (0 if file <= 4GB)
    Utils::RegWrite(deviceType, 0x128, write_size_hi);

    quint64 ReadFileSize = static_cast<quint64>(fileInfo.size());
    //Split into two 32-bit parts
    size_lo = static_cast<quint32>(ReadFileSize & 0xFFFFFFFFULL);
    size_hi = static_cast<quint32>((ReadFileSize >> 32) & 0xFFFFFFFFULL);

    //Write start pulse
    quint32 value = (1u << 10);
    Utils::RegWrite(deviceType, 0x118, value);
    value = 0;
    Utils::RegWrite(deviceType, 0x118, value);

    FileReadWriteSetup(deviceType,fileSize,filename,eWrite);
}
void MainWindow::SetDAC1ReadSettingAfterFileSend()
{
    uint32_t val;
    int retries = 5000;

    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("[WriteRegister] Interface not selected");
        return;
    }
    //Write start pulse
    Utils::RegWrite(deviceType, 0x210C,0x00); //Start address
    Utils::RegWrite(deviceType, 0x2110,0x00); //Start address

    Utils::RegWrite(deviceType, 0x2114,size_lo);
    Utils::RegWrite(deviceType, 0x212C,size_hi);

    Utils::RegWrite(deviceType, 0x2118,0x200);

    do {
        val = Utils::RegRead(deviceType, 0x11C);
        QThread::msleep(1);
        LOG_INFO("Verifying write done %d",retries);
    } while (val != 0x03 && retries-- > 0);

    if (val == 0x03)
        Log::showStatusMessage(this, "Write status", "Write successful");
    else
        Log::showStatusMessage(this, "Write status", "Write failed");

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
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("[WriteRegister] Interface not selected");
        return;
    }

    Utils::RegWrite(deviceType,0x5000,0x4);// ddr3 reset
    Utils::RegWrite(deviceType,0x5000,0x0);

    int iPwSample = ui->SbDAC1PWSamples->value();
    int iSignalDelay  = ui->SbDAC1SignalDelay->value();
    int iDoplerShiftHz = ui->SbDAC1DopplerShiftHz->value();

    DacHelper::WrIterpolation(deviceType, ui->CbIDAC1InterpSelect->currentText().toInt());
    DacHelper::WrNCOFrq(deviceType,ui->CbDAC1NOCFrequency->currentText());

    Utils::RegWrite(deviceType,AVR_DAC3_BASE_ADDR+0x08,iPwSample);
    Utils::RegWrite(deviceType,AVR_DAC3_BASE_ADDR+0x6C,iSignalDelay);
    Utils::RegWrite(deviceType,AVR_DAC3_BASE_ADDR+0x40,iDoplerShiftHz);

    Utils::RegWrite(deviceType,0x534,0x1);  // trigger start
    Utils::RegWrite(deviceType,0x534,0x0);
    Utils::RegWrite(deviceType,0x2018,0x1);// trigger enable
    Utils::RegWrite(deviceType,0x2200,0x2);// DAC selection

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
        Utils::RegWrite(eETHPL1G,0x508,1);
    }
    else{
        Utils::RegWrite(eETHPL1G,0x508,0);
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
    Utils::RegWrite(eETHPL1G,0x504,totalval);
    Utils::RegWrite(eETHPL1G,0x508,3);
    Utils::RegWrite(eETHPL1G,0x508,1);
}

void MainWindow::on_PbDAC1IQFileBrowse_clicked()
{
    QString filename = QFileDialog::getOpenFileName(
        this,
        tr("Open Binary File"),
        QString(),
        tr("Binary Files (*.bin);;All Files (*.*)")
        );

    if(!filename.isEmpty()){
        ui->LeDAC1IQFileName->setText(filename);
    }

}


void MainWindow::on_PbDAC1Start_clicked()
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("[WriteRegister] Interface not selected");
        return;
    }
    Utils::RegWrite(deviceType,AVR_DAC3_BASE_ADDR+0x018,0x1);
    Utils::RegWrite(deviceType,AVR_DAC3_BASE_ADDR+0x010,0x1);
    Utils::RegWrite(deviceType,AVR_DAC3_BASE_ADDR+0x014,0x1);
}


void MainWindow::on_PbDAC1Stop_clicked()
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("[WriteRegister] Interface not selected");
        return;
    }
    // Utils::RegWrite(deviceType,AVR_DAC3_BASE_ADDR+0x014,0x1);
    // Utils::RegWrite(deviceType,AVR_DAC3_BASE_ADDR+0x014,0x0);
    // Utils::RegWrite(deviceType,0x2118,0x0);
    // Utils::RegWrite(deviceType,0x2158,0x0);
    // Utils::RegWrite(deviceType,0x5000,0x8);
    // Utils::RegWrite(deviceType,0x5000,0xC);
    // Utils::RegWrite(deviceType,0x5000,0x4);
    // Utils::RegWrite(deviceType,0x5000,0x0);
    // Utils::RegWrite(deviceType,0x2118,0x200);
    // Utils::RegWrite(deviceType,0x2158,0x200);




    Utils::RegWrite(deviceType,AVR_DAC3_BASE_ADDR+0x010,0x0);
    Utils::RegWrite(deviceType,AVR_DAC3_BASE_ADDR+0x014,0x0);
    Utils::RegWrite(deviceType,0x2118,0x0);
    Utils::RegWrite(deviceType,0x2158,0x0);
    Utils::RegWrite(deviceType,0x5000,0x8);
    Utils::RegWrite(deviceType,0x5000,0xC);
    Utils::RegWrite(deviceType,0x5000,0x4);
    Utils::RegWrite(deviceType,0x5000,0x0);
    Utils::RegWrite(deviceType,0x2118,0x200);
    Utils::RegWrite(deviceType,0x2158,0x200);
}


void MainWindow::on_PbDAC1Restart_clicked()
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("[WriteRegister] Interface not selected");
        return;
    }
    Utils::RegWrite(deviceType,AVR_DAC3_BASE_ADDR+0x018,0x1);
    Utils::RegWrite(deviceType,AVR_DAC3_BASE_ADDR+0x010,0x1);
    Utils::RegWrite(deviceType,AVR_DAC3_BASE_ADDR+0x014,0x1);
}


void MainWindow::on_PbDAC1FIFOEntryErrorUpdate_clicked()
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("[WriteRegister] Interface not selected");
        return;
    }
    Utils::RegWrite(deviceType,0x5000,0x80);
    Utils::RegWrite(deviceType,0x5000,0x00);
    Utils::RegWrite(deviceType,0x5004,0xF000);
    Utils::RegWrite(deviceType,0x5000,0x100);
    Utils::RegWrite(deviceType,0x5000,0x00);
    Utils::RegWrite(deviceType,0x5000,0x80);
    Utils::RegWrite(deviceType,0x5000,0x00);
    Utils::RegWrite(deviceType,0x5004,0xF000);
    Utils::RegWrite(deviceType,0x5000,0x100);
    Utils::RegWrite(deviceType,0x5000,0x00);

    uint32_t ReadVal = Utils::SpiDacRead(deviceType,0x05,0x02);
    LOG_INFO("PbDAC1FIFOEntryErrorUpdate:Read Val %d",ReadVal);
    Utils::SpiDacWrite(deviceType,0x05,0x00,0x2);
    ReadVal = Utils::SpiDacRead(deviceType,0x05,0x02);
    LOG_INFO("PbDAC1FIFOEntryErrorUpdate : After write Read Val %d",ReadVal);
}




void MainWindow::on_RbPL10GSel_clicked()
{

}


void MainWindow::on_RbPL10GSel_clicked(bool checked)
{

}


void MainWindow::on_CbDAC1NOCFrequency_currentTextChanged(const QString &arg1)
{

}


void MainWindow::on_CbDAC1NOCFrequency_currentIndexChanged(int index)
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("[WriteRegister] Interface not selected");
        return;
    }

    if(ui->ChkBoxDAC1NOCEnable->isChecked())
    {
        if( index == 0){
            DacHelper::NCO_FRQ60Mhz(deviceType);
        }
        else if( index == 1){
            DacHelper::NCO_FRQ70Mhz(deviceType);
        }
        else if( index == 2){
            DacHelper::NCO_FRQ180Mhz(deviceType);
        }
    }
}

