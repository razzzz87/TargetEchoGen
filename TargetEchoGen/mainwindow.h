#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "connectiontype.h"
#include <QMainWindow>
#include <QGroupBox>
#include <udpcon.h>
#include "FileTransferAgent.h"
#include <QTimer>

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
    UDP_PS1G_Con *EthPs01G;
    UDP_PL10G_Con *EthPl10G;
    UDP_PL1G_Con *EthPl01G;
    FileTransferAgent  *setupTransferAgent;

    void load_files();

private slots:
    void on_pushButton_conn_settings_clicked();
    void on_pushButton_ddr_if_file_browse_clicked();
    void on_pushButton_ddr_if_amplitude_file_browse_clicked();
    void on_pushButton__ddr_if_amplitude_file_send_clicked();
    void on_pushButton_ddr_if_dac1_send_clicked();
    void on_pushButton_ddr_lx_file_browse_clicked();
    void on_pushButton_ddr_dac_iq_file_browse_clicked();
    void on_pushButton_ddr_dac_iq_file_send_clicked();
    void onTimeout();  // ✅ This is your slot triggered by the timer
    void updateTransferProgress(qint64 percentage);
    void close_Progress_pop(void);
private:
    Ui::MainWindow *ui;
    QProgressDialog* transferProgress = nullptr;
    bool transferCanceled = false;
};

// class MainWindow : public QMainWindow
// {
//     Q_OBJECT

// public:
//     MainWindow(QWidget *parent = nullptr);
//     ~MainWindow();

//     QWidget *file_processing;
//     QWidget *device_setup;
//     QGroupBox *ddr_groupbox;
//     ConnectionType *conn;
//     UDP_PS1G_Con *EthPs01G;
//     UDP_PL10G_Con *EthPl10G;
//     UDP_PL1G_Con *EthPl01G;
//     FileTransferAgent  *MainUDPSender;

//     void load_files();
// private slots:
//     void on_pushButton_conn_settings_clicked();

//     void on_pushButton_ddr_if_file_browse_clicked();

//     void on_pushButton_ddr_if_amplitude_file_browse_clicked();

//     void on_pushButton__ddr_if_amplitude_file_send_clicked();

//     void on_pushButton_ddr_if_dac1_send_clicked();

//     void on_pushButton_ddr_lx_file_browse_clicked();

//     void on_pushButton_ddr_dac_iq_file_browse_clicked();

//     void on_pushButton_ddr_dac_iq_file_send_clicked();

// private slots:
//     void onTimeout();

// private:
//     Ui::MainWindow *ui;
//     QTime ProgressbarTimer;
// };
#endif // MAINWINDOW_H
