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


    relay = new PacketForwarder(
        "0.0.0.0",    // UDP receive (IMS) bind IP (any)
        0xD407,       // UDP receive port
        "10.0.0.80",  // UDP TX target IP (FPGA / delay receiver)
        4660,         // UDP TX target port
        "relay_log.csv",
        this
        );

    if (!relay->initialize()) {
        LOG_ERROR("[FileProcessing] Socket creation failed");
    }
}

FileProcessing::~FileProcessing()
{
    delete ui;
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
    relay->SendCoOrdinateOverTcp(deviceType);
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
    }
}


void FileProcessing::on_PbFPCSVFileSend_clicked()
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("[WriteRegister] Interface not selected");
        return;
    }

    if(!ui->LePFCSVFilePath->text().isEmpty())
        relay->sendCoordinateFileUdp(ui->LePFCSVFilePath->text());
    else{
        Log::showStatusMessage(this,"File Processing","Please select CSV File");
    }

}

