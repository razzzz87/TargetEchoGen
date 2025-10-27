#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "connectiontype.h"
#include <QMainWindow>
#include <QGroupBox>
#include <udpcon.h>
#include "FileTransferAgent.h"
#include <QTimer>
#include "devicesetup.h"
#include "fileprocessing.h"
#include "rf.h"
#include "selftest.h"
#include "spectrum.h"
#include "transferprogressdialog.h"
#include "uartserial.h"
#include "Utils.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QWidget *file_processing;
    QWidget *device_setup;
    QGroupBox *ddr_groupbox;
    ConnectionType *conn;
    UartSerial *serial;

    DeviceSetup* deviceSetup;
    FileProcessing* fileProcessing;
    SelfTest* selfTest;
    Spectrum* spectrum;
    RF* rf;

    TransferProgressDialog* progressDialog;
    FileTransferAgent  *setupTransferAgent;
    void load_files();
    void FileReadWriteSetup(iface deviceType, uint iFileSize, QString sFilePath, eXferDir dir);
    uint64_t ComputeDDSFCW(uint32_t fcw, uint32_t fs);

private slots:
    void onConnStateChanged(iface which, ConnInfo s);
    void onTimeout();
    void updateTransferProgress(qint64 percentage);
    void close_Progress_pop(void);
    void on_PbConnSettings_clicked();
    void onConnectionSuccess(iface eInterface);
    void onConnectionFailure(iface eInterface);
    void on_PbDAC1IQFileSend_clicked();

    void on_PbDAC2TgrSetup_clicked();

    void on_PbDAC2TgrStart_clicked();

    void on_PbDAC2TgrStop_clicked();

    void on_PbDAC1AMPLFixedLevelSet_clicked();

    void on_ChkBoxDAC1NOCEnable_checkStateChanged(const Qt::CheckState &arg1);

    void on_PbDAC1TriggerSetup_clicked();

    void on_PBdac1TSstart_clicked();

    void on_PBdac1TSstop_clicked();

    void on_PbDAC2AMPLFixedLevelSet_clicked();

    void on_PbDAC1AMPLBaseValueIncr_clicked();

    void on_PbDAC1AMPLBaseValueDecr_clicked();

    void on_PbDAC2AMPLBaseValueIncr_clicked();

    void on_PbDAC2AMPLBaseValueDncr_clicked();

    void on_PbDAC1Apply_clicked();

    void on_PbDAC1SUMRefresh_clicked();

    void on_ChkBoxNBADCDDSEnable_checkStateChanged(const Qt::CheckState &arg1);

    void on_PbNB_ADC_DDSFCWSet_clicked();
signals:
    void resizeEventTriggered();

private:
    Ui::MainWindow *ui;
    bool transferCanceled = false;
};
#endif // MAINWINDOW_H
