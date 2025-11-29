#include "devicesetup.h"
#include "ui_devicesetup.h"
#include <QMessageBox>
#include <QHostAddress>
#include "log.h"
#include "proto.h"
#include "FileTransferAgent.h"
#include "Utils.h"
#include "devicesetuphelper.h"
#include "connectionctx.h"

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

    //If the helper is inside DeviceSetupHelper:
    for (int i = 0; i < static_cast<int>(DeviceType::COUNT); ++i)
    {
        DeviceType d = static_cast<DeviceType>(i);
        ui->CbDevType->addItem(DeviceSetupHelper::DeviceTypeToQStringDirect(d),QVariant(static_cast<int>(d)));
    }

}

DeviceSetup::~DeviceSetup()
{
    delete ui;
}

iface DeviceSetup::getSelectedDeviceType()
{
    auto& ctx = ConnectionHelper::instance();
    iface sel = ctx.selectedInterface();

    const QString ifaceName = Utils::ifaceToQString(sel);

    // 1️⃣ Check if no interface selected
    if (sel == eNONE)
    {
        LOG_ERROR("[DeviceSetup] No interface selected (iface=%s)", Utils::ifaceToCStr(sel));
        Log::showStatusMessage(this, "Device Setup", "Please select an interface before proceeding.");
        return eNONE;
    }

    // 2️⃣ Check if selected interface is connected
    ConnInfo info = ctx.info(sel);
    if (!info.connected)
    {
        LOG_ERROR("[DeviceSetup] Selected interface '%s' is NOT connected.", Utils::ifaceToCStr(sel));
        Log::showStatusMessage(this, "Device Setup",
                               QString("Selected interface '%1' is not connected.").arg(ifaceName));
        return eNONE;
    }

    // 3️⃣ Success — valid and connected interface
    LOG_INFO("[DeviceSetup] Selected and connected interface: %s", Utils::ifaceToCStr(sel));
    //Log::showStatusMessage(this, "Device Setup", QString("Selected Interface: %1").arg(ifaceName));

    return sel;
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

void DeviceSetup::WriteRegisterAndShow(QLineEdit *leAddr, QLineEdit *leVal)
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("[WriteRegister] Interface not selected");
        return;
    }
    const QString addrStr = leAddr->text().trimmed();
    const QString valStr  = leVal->text().trimmed();

    // Try parse now so entry log can show numeric hex too
    bool ok = false;
    const uint32_t parsedAddr = addrStr.toUInt(&ok, 16);
    const bool addrOk = ok;
    ok = false;
    const uint32_t parsedVal  = valStr.toUInt(&ok, 16);
    const bool valOk = ok;

    // Entry log with raw text and parsed hex (0xFFFFFFFF when parse fails)
    LOG_INFO("[WriteRegister] <ENTER>: addrStr=%s valStr=%s parsedAddr=0x%08X parsedVal=0x%08X",
             addrStr.toStdString().c_str(),
             valStr.toStdString().c_str(),
             addrOk ? parsedAddr : 0xFFFFFFFF,
             valOk  ? parsedVal  : 0xFFFFFFFF);

    if (!addrOk) {
        LOG_ERROR("[WriteRegister] Invalid address string: %s", addrStr.toStdString().c_str());
        return;
    }
    if (!valOk) {
        LOG_ERROR("[WriteRegister] Invalid value string: %s", valStr.toStdString().c_str());
        return;
    }

    const uint32_t addr = parsedAddr;
    const uint32_t value = parsedVal;

    // Parsed numeric forms logged as hex
    LOG_INFO("[WriteRegister] Parsed <ENTER>: addr=0x%08X val=0x%08X", addr, value);

    // Read DeviceType enum from combo itemData (assumes you stored enum as int)
    QVariant data = ui->CbDevType->currentData();
    const int devInt = data.isValid() ? data.toInt() : -1;
    const DeviceType dev = (devInt >= 0 && devInt < static_cast<int>(DeviceType::COUNT))
                               ? static_cast<DeviceType>(devInt)
                               : DeviceType::COUNT;

    // Dispatch write based on enum
    if (dev == DeviceType::LMX)
    {
        //Utils::WriteSpiSynth(deviceType, addr, value);
        //LOG_INFO("[WriteRegister] Performed LMX SPI write addr=0x%08X val=0x%08X", addr, value);
    }
    else if (dev == DeviceType::LMK)
    {
        Utils::WriteSpiSynth(deviceType, addr, value);
    }
    else if (dev == DeviceType::FPGA)
    {
        Utils::RegWrite(deviceType, addr, value);
    }
    else if (dev == DeviceType::DAC1)
    {
        Utils::SpiDacWrite(deviceType, addr, value, 0x00);
    }
    else if (dev == DeviceType::DAC2)
    {
        Utils::SpiDacWrite(deviceType, addr, value, 0x01);
    }
    else if (dev == DeviceType::DAC3)
    {
        Utils::RegWrite(deviceType, addr, value);
    }
    else if (dev == DeviceType::ATTN1)
    {
        //Utils::SpiDacWrite(deviceType, addr, value, 0x03);
        //LOG_INFO("[WriteRegister] Performed ATTN1 SPI write addr=0x%08X val=0x%08X", addr, value);
    }
    else if (dev == DeviceType::ATTN2)
    {
        //Utils::SpiDacWrite(deviceType, addr, value, 0x04);
        //LOG_INFO("[WriteRegister] Performed ATTN2 SPI write addr=0x%08X val=0x%08X", addr, value);
    }
    else if (dev == DeviceType::ATTN3)
    {
        //Utils::SpiDacWrite(deviceType, addr, value, 0x05);
        //LOG_INFO("[WriteRegister] Performed ATTN3 SPI write addr=0x%08X val=0x%08X", addr, value);
    }
    else if (dev == DeviceType::ATTN4)
    {
        //Utils::SpiDacWrite(deviceType, addr, value, 0x06);
        //LOG_INFO("[WriteRegister] Performed ATTN4 SPI write addr=0x%08X val=0x%08X", addr, value);
    }
    else
    {
        Utils::RegWrite(deviceType, addr, value);
    }

    // Update UI to normalized hex string and final exit log
    leVal->setText(QString("%1").arg(value, 8, 16, QChar('0')).toUpper());
    LOG_INFO("[WriteRegister] <EXIT>: addr=0x%08X val=0x%08X", addr, value);
}


void DeviceSetup::ReadRegisterAndShow(QLineEdit *leAddr, QLineEdit *leVal, const QString &devLabel, uint8_t dacPage)
{
    LOG_INFO("[ReadRegister] <ENTER>: devLabel=%s", devLabel.toStdString().c_str());

    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("[ReadRegister] Interface not selected");
        return;
    }

    bool ok = false;
    uint32_t uiAddr = leAddr->text().toUInt(&ok,16);
    if (!ok) {
        LOG_ERROR("[ReadRegister] Invalid address: %s", leAddr->text().toStdString().c_str());
        return;
    }

    const QString s = devLabel.trimmed().toUpper();
    uint32_t uiRegVal = 0;

    if (s == "LMK")
    {
        uiRegVal = Utils::ReadSpiSynth(deviceType, uiAddr);
    }
    else if (s == "LMX")
    {
        uiRegVal = Utils::ReadSpiSynth(deviceType, uiAddr); // adjust if LMX uses different API
    }
    else if (s == "FPGA")
    {
        Utils::readRegisterValue(deviceType, leAddr, leVal);
        LOG_INFO("[ReadRegister] <EXIT>: delegated to readRegisterValue for FPGA");
        return;
    }
    else if (s == "DAC1")
    {
        uiRegVal = Utils::SpiDacRead(deviceType, uiAddr, 0x00);
    }
    else if (s == "DAC2")
    {
        uiRegVal = Utils::SpiDacRead(deviceType, uiAddr, 0x01);
    }
    else if (s == "DAC3")
    {
        uiRegVal = Utils::RegRead(deviceType,uiAddr);
    }
    else if (s == "ATTN1")
    {
        //uiRegVal = Utils::SpiDacRead(deviceType, uiAddr, dacPage ? dacPage : 0x03);
    }
    else if (s == "ATTN2")
    {
        //uiRegVal = Utils::SpiDacRead(deviceType, uiAddr, dacPage ? dacPage : 0x04);
    }
    else if (s == "ATTN3")
    {
        //uiRegVal = Utils::SpiDacRead(deviceType, uiAddr, dacPage ? dacPage : 0x05);
    }
    else if (s == "ATTN4")
    {
        //uiRegVal = Utils::SpiDacRead(deviceType, uiAddr, dacPage ? dacPage : 0x06);
    }
    else {
        Utils::readRegisterValue(deviceType, leAddr, leVal);
        LOG_INFO("[ReadRegister] <EXIT>: delegated to readRegisterValue (unknown device)");
        return;
    }
    leVal->setText(QString("%1").arg(uiRegVal, 8, 16, QChar('0')).toUpper());
    LOG_INFO("[ReadRegister] Read addr=0x%X val=0x%08X dev=%s", uiAddr, uiRegVal, devLabel.toStdString().c_str());
    LOG_INFO("[ReadRegister] <EXIT>: devLabel=%s", devLabel.toStdString().c_str());
}

void DeviceSetup::on_PbRegRead1_clicked()
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE){
        LOG_ERROR("Interface not selected %d",deviceType);
        return;
    }
    ReadRegisterAndShow(ui->LeRegReadAddr1, ui->LeRegReadVal1,ui->CbDevType->currentText());
}

void DeviceSetup::on_PbRegRead2_clicked()
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE){
        LOG_ERROR("Interface not selected %d",deviceType);
        return;
    }
    ReadRegisterAndShow(ui->LeRegReadAddr2, ui->LeRegReadVal2,ui->CbDevType->currentText());
}

void DeviceSetup::on_PbRegRead3_clicked()
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE){
        LOG_ERROR("Interface not selected %d",deviceType);
        return;
    }
    ReadRegisterAndShow(ui->LeRegReadAddr3, ui->LeRegReadVal3,ui->CbDevType->currentText());
}

void DeviceSetup::on_PbRegRead4_clicked()
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE){
        LOG_ERROR("Interface not selected %d",deviceType);
        return;
    }
    ReadRegisterAndShow(ui->LeRegReadAddr4, ui->LeRegReadVal4,ui->CbDevType->currentText());
}

void DeviceSetup::on_PbRegWrite1_clicked()
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE){
        LOG_ERROR("Interface not selected %d",deviceType);
        return;
    }
    WriteRegisterAndShow(ui->LeRegWriteAddr1, ui->LeRegWriteVal1);
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
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE){
        LOG_ERROR("Interface not selected %d",deviceType);
        return;
    }
    FileReadWriteSetup(deviceType,ui->LeMemReadFileNameReadSize->text().toInt(),ui->LeMemReadFileNamePath->text(),eRead);
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
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE){
        LOG_ERROR("Interface not selected %d",deviceType);
        return;
    }
    FileReadWriteSetup(deviceType,ui->LeMemWriteFileSize->text().toInt(),ui->LeMemWriteFileNamePath->text(),eWrite);
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
    Log::showStatusMessage(this,"Device setup","LMK Default setting done");
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
        DeviceSetupHelper::Dac3DefaultSetting(deviceType);
    }
    else {}
    Log::showStatusMessage(this,"Device setup","DAC Init Default setting done");
    LOG_INFO("DeviceSetup::on_PbDACInitDefault_clicked() <EXIT>\n");
}

void DeviceSetup::on_PbRegWrite2_clicked()
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE){
        LOG_ERROR("Interface not selected %d",deviceType);
        return;
    }
    WriteRegisterAndShow(ui->LeRegWriteAddr2, ui->LeRegWriteVal2);
}


void DeviceSetup::on_PbRegWrite3_clicked()
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE){
        LOG_ERROR("Interface not selected %d",deviceType);
        return;
    }
    WriteRegisterAndShow(ui->LeRegWriteAddr3, ui->LeRegWriteVal3);
}


void DeviceSetup::on_PbRegWrite4_clicked()
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE){
        LOG_ERROR("Interface not selected %d",deviceType);
        return;
    }
    WriteRegisterAndShow(ui->LeRegWriteAddr4, ui->LeRegWriteVal4);
}

