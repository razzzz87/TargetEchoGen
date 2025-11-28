#include "fileprocessing.h"
#include "Utils.h"
#include "ui_fileprocessing.h"
#include "connectionctx.h"
#include "log.h"

FileProcessing::FileProcessing(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FileProcessing)
{
    ui->setupUi(this);

    ui->LeFPTPXAxis->setValidator(new QDoubleValidator(-5000, 5000, 6, this));
    ui->LeFPTPYAxis->setValidator(new QDoubleValidator(-5000, 5000, 6, this));
    ui->LeFPTPZAxis->setValidator(new QDoubleValidator(-5000, 5000, 6, this));


    relay = new PacketForwarder(
    "127.0.0.1",      // src server IP (IMS server / server.py)
    0xD407,           // src server port
    "192.168.1.50",   // dst server IP (where you forward)
    6000,             // dst server port
    "relay_log.csv",  // CSV log
    this
    );
    //relay->setTarget(10.0, 10.0, 10.0);   // Xtp, Ytp, Ztp
    // relay->start();
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
        Log::showStatusMessage(this, "Device Setup",
                               QString("Selected interface '%1' is not connected.").arg(ifaceName));
        return eNONE;
    }

    // 3️⃣ Success — valid and connected interface
    LOG_INFO("[FileProcessing] Selected and connected interface: %s", Utils::ifaceToCStr(sel));

    return sel;
}

void FileProcessing::on_PbFPTargetPostionSet_clicked()
{
    bool okX = false, okY = false, okZ = false;

    double Xtp = ui->LeFPTPXAxis->text().toDouble(&okX);
    double Ytp = ui->LeFPTPYAxis->text().toDouble(&okY);
    double Ztp = ui->LeFPTPZAxis->text().toDouble(&okZ);

    if (!okX || !okY || !okZ) {
        Log::showStatusMessage(this, "Invalid Input","Please enter valid numeric values for X, Y, Z.");
        return;
    }

    relay->setTarget(Xtp, Ytp, Ztp);
}

