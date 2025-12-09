#include "fileprocessing.h"
#include "Utils.h"
#include "ui_fileprocessing.h"
#include "connectionctx.h"
#include "log.h"
#include "QFileDialog"

FileProcessing::FileProcessing(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FileProcessing)
{
    ui->setupUi(this);

    ui->LeFPTPXAxis->setValidator(new QDoubleValidator(-5000, 5000, 6, this));
    ui->LeFPTPYAxis->setValidator(new QDoubleValidator(-5000, 5000, 6, this));
    ui->LeFPTPZAxis->setValidator(new QDoubleValidator(-5000, 5000, 6, this));
    ui->GrpBoxFPRealTime->setChecked(false);

    relay = new PacketForwarder(
        "0.0.0.0",    // UDP receive (IMS) bind IP (any)
        54279,        // UDP receive port
        "10.0.0.80",  // UDP TX target IP (FPGA / delay receiver)
        4660,         // UDP TX target port
        "relay_log.csv",
        this
        );

    if (!relay->initialize()) {
        LOG_ERROR("[FileProcessing] Socket creation failed");
    }
    connect(relay, &PacketForwarder::measurementUpdated,this,  &FileProcessing::onMeasurementUpdated);
}

FileProcessing::~FileProcessing()
{
    delete ui;
}

void FileProcessing::onMeasurementUpdated(const RelayMeasurement &m)
{
    // // Example field names – adjust to your actual UI names
     ui->LelFPRTXAxis->setText(QString::number(m.Xtp, 'f', 3));
     ui->LelFRTYAxis->setText(QString::number(m.Ytp, 'f', 3));
     ui->LelFPRTZAxis->setText(QString::number(m.Ztp, 'f', 3));
     ui->LeFPRTDelay->setText(QString::number(m.delay_us, 'f', 3));

    // ui->LePosX->setText(QString::number(m.x, 'f', 3));
    // ui->LePosY->setText(QString::number(m.y, 'f', 3));
    // ui->LePosZ->setText(QString::number(m.z, 'f', 3));

    // ui->LeDistance->setText(QString::number(m.dist_m, 'f', 3));
    // ui->LeDelayUs->setText(QString::number(m.delay_us));

    // ui->LeMsgNum->setText(QString::number(m.msg_num));
    // ui->LeIdField->setText(QString("0x%1").arg(m.id_field, 8, 16, QLatin1Char('0')).toUpper());

    // if you want to show startTime:
    // ui->LeStartTime->setText(QString::number(m.startTime, 'f', 6));
}
iface FileProcessing::getSelectedDeviceType()
{
    auto& ctx = ConnectionHelper::instance();
    iface sel = ctx.selectedInterface();

    const QString ifaceName = Utils::ifaceToQString(sel);

    // 1️⃣ Check if no interface selected
    if (sel == eNONE)
    {
        LOG_ERROR("[FileProcessing] No interface selected (iface=%s)", Utils::ifaceToCStr(sel));
        Log::showStatusMessage(this, "Device Setup", "Please select an interface before proceeding.");
        return eNONE;
    }

    // 2️⃣ Check if selected interface is connected
    ConnInfo info = ctx.info(sel);
    if (!info.connected)
    {
        LOG_ERROR("[FileProcessing] Selected interface '%s' is NOT connected.", Utils::ifaceToCStr(sel));
        Log::showStatusMessage(this, "Device Setup", QString("Selected interface '%1' is not connected.").arg(ifaceName));
        return eNONE;
    }

    // 3️⃣ Success — valid and connected interface
    LOG_INFO("[FileProcessing] Selected and connected interface: %s", Utils::ifaceToCStr(sel));

    return sel;
}

void FileProcessing::on_PbFPTargetPostionSet_clicked()
{
    bool okX = false, okY = false, okZ = false;

    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("[WriteRegister] Interface not selected");
        return;
    }
    //relay->m_startTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
    double Xtp = ui->LeFPTPXAxis->text().toDouble(&okX);
    double Ytp = ui->LeFPTPYAxis->text().toDouble(&okY);
    double Ztp = ui->LeFPTPZAxis->text().toDouble(&okZ);

    if (!okX || !okY || !okZ) {
        Log::showStatusMessage(this, "Invalid Input","Please enter valid numeric values for X, Y, Z.");
        return;
    }
    relay->setTarget(Xtp, Ytp, Ztp);
    //relay->SendCoOrdinateOverTcp(deviceType);
    //relay->send_delay_once(Xtp, Ytp, Ztp);
}


void FileProcessing::on_PbFPFileBrowse_clicked()
{
    QString filename = QFileDialog::getOpenFileName(
        this,
        tr("Open Binary File"),
        QString(),
        tr("Binary Files (*.bin);;All Files (*.*)")
        );

    if(!filename.isEmpty()){
        ui->LePFCSVFilePath->setText(filename);
        if(!simulateCoordinateFileFits(ui->LePFCSVFilePath->text()))
            Log::showStatusMessage(this,"File processing","File data not multiple of 1024.");
    }
}
bool  FileProcessing::simulateCoordinateFileFits(const QString &filePath)
{
    const int WORD_SIZE = 4;
    const int PACKET_DATA_SIZE = ETH_DATA_SIZE;   // required size

    QFile file(filePath);
    if (!file.exists()) {
        LOG_ERROR("simulate: file does not exist: %s", filePath.toStdString().c_str());
        return false;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOG_ERROR("simulate: cannot open file %s (error: %s)",
                  filePath.toStdString().c_str(),
                  file.errorString().toStdString().c_str());
        return false;
    }

    QTextStream in(&file);

    qint64 lineNo = 0;
    qint64 totalWords = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNo++;

        if (line.isEmpty() || line.startsWith('#'))
            continue;

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        QStringList tok = line.split(QRegularExpression("[,\\s]+"), Qt::SkipEmptyParts);
#else
        QStringList tok = line.split(QRegExp("[,\\s]+"), QString::SkipEmptyParts);
#endif

        if (tok.size() < 4) {
            LOG_ERROR("simulate: line %lld malformed (expected 4 columns): '%s'",
                      (long long)lineNo, line.toStdString().c_str());
            return false;
        }

        bool ok0, ok1, ok2, ok3;
        tok[0].toUInt(&ok0, 10);
        tok[1].toUInt(&ok1, 16);
        tok[2].toUInt(&ok2, 16);
        tok[3].toUInt(&ok3, 16);

        if (!(ok0 && ok1 && ok2 && ok3)) {
            LOG_ERROR("simulate: line %lld parsing failure: '%s'",
                      (long long)lineNo, line.toStdString().c_str());
            return false;
        }

        totalWords++;  // one 32-bit word per CSV row
    }

    const qint64 totalBytes = totalWords * WORD_SIZE;

    LOG_INFO("simulate: file has %lld words (%lld bytes)",(long long)totalWords, (long long)totalBytes);

    // ---- FINAL VALIDATION ----
    if (totalBytes % PACKET_DATA_SIZE != 0) {
        LOG_ERROR("simulate: totalBytes (%lld) is NOT a multiple of %d bytes.",(long long)totalBytes, PACKET_DATA_SIZE);
        LOG_ERROR("simulate: This file CANNOT be split into perfect 1024-byte packets.");
        return false;
    }

    LOG_INFO("simulate: VALID — file fits perfectly into packets of 1024 bytes.");
    return true;
}


void FileProcessing::on_PbFPCSVFileSend_clicked()
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("[WriteRegister] Interface not selected");
        return;
    }

    if(!ui->LePFCSVFilePath->text().isEmpty()){
        relay->sendCoordinateFileUdp(ui->LePFCSVFilePath->text());
        Utils::RegWrite(deviceType,0x2078,0x01);
        Utils::RegWrite(deviceType,0x2074,0x01);
    }
    else{
        Log::showStatusMessage(this,"File Processing","Please select CSV File");
    }

}


void FileProcessing::on_GRpBoxUserInput_clicked(bool checked)
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("[WriteRegister] Interface not selected");
        return;
    }

    if(checked){

        ui->GrpBoxFPCsvFileSend->setChecked(false);
        ui->GrpBoxFPRealTime->setChecked(false);
        Utils::RegWrite(deviceType,0x2078,0x00);
        Utils::RegWrite(deviceType,0x2074,0x00);
        relay->stop();
    }

}


void FileProcessing::on_GrpBoxFPRealTime_clicked(bool checked)
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("[WriteRegister] Interface not selected");
        return;
    }

    if(checked){

        ui->GrpBoxFPCsvFileSend->setChecked(false);
        ui->GRpBoxUserInput->setChecked(false);
        Utils::RegWrite(deviceType,0x2078,0x00);
        Utils::RegWrite(deviceType,0x2074,0x00);
        relay->start();

    }else{
        relay->stop();
    }
}


void FileProcessing::on_GrpBoxFPCsvFileSend_clicked(bool checked)
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("[WriteRegister] Interface not selected");
        return;
    }

    if(checked){

        ui->GrpBoxFPRealTime->setChecked(false);
        ui->GRpBoxUserInput->setChecked(false);
        Utils::RegWrite(deviceType,0x2078,0x00);
        Utils::RegWrite(deviceType,0x2074,0x00);
        relay->stop();
    }
}


void FileProcessing::on_PbFPTargetVelocitySet_clicked()
{

}


void FileProcessing::on_GRpBoxUserInput_clicked()
{

}

