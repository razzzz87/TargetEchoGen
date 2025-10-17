#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "connectiontype.h"
#include <QMainWindow>
#include <QGroupBox>
#include <udpcon.h>
#include "FileTransferAgent.h"
#include <QTimer>
#include "transferprogressdialog.h"
#include "uartserial.h"

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
    TransferProgressDialog* progressDialog;
    FileTransferAgent  *setupTransferAgent;
    void load_files();
    void FileReadWriteSetup(iface deviceType, uint iFileSize, QString sFilePath, eXferDir dir);

private slots:

    void onTimeout();
    void updateTransferProgress(qint64 percentage);
    void close_Progress_pop(void);
    void on_PbConnSettings_clicked();
    void onConnectionSuccess(iface eInterface);
    void onConnectionFailure(iface eInterface);
    void on_PbDAC1IQFileSend_clicked();

    void on_PBdac2TStriggersetup_clicked();

    void on_PbDAC2TgrSetup_clicked();

    void on_PbDAC2TgrStart_clicked();

    void on_PbDAC2TgrStop_clicked();

    void on_PbDAC1AMPLFixedLevelSet_clicked();

    void on_ChkBoxDAC1NOCEnable_checkStateChanged(const Qt::CheckState &arg1);

    void on_PbDAC1TriggerSetup_clicked();

    void on_PBdac1TSstart_clicked();

    void on_PBdac1TSstop_clicked();

    void on_CbDAC2AMPLFixedLevelSet_clicked();

    void on_PbDAC2AMPLFixedLevelSet_clicked();

    void on_PbDAC1AMPLBaseValueIncr_clicked();

    void on_PbDAC1AMPLBaseValueDecr_clicked();

    void on_PbDAC2AMPLBaseValueIncr_clicked();

private:
    Ui::MainWindow *ui;
    bool transferCanceled = false;
};
#endif // MAINWINDOW_H
