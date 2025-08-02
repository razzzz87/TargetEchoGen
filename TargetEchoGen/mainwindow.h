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
//protected:
    //void resizeEvent(QResizeEvent *event) override;
private slots:
    //void on_pushButton_conn_settings_clicked();
    //void on_pushButton_ddr_if_amplitude_file_browse_clicked();
    //void on_pushButton__ddr_if_amplitude_file_send_clicked();
    //void on_pushButton_ddr_lx_file_browse_clicked();
    //void on_pushButton_ddr_dac_iq_file_browse_clicked();
    //void on_pushButton_ddr_dac_iq_file_send_clicked();
    void onTimeout();
    void updateTransferProgress(qint64 percentage);
    void close_Progress_pop(void);
    void on_pb_conn_settings_clicked();

    void on_pb_refresh_button_clicked();

    void on_pb_ddr_dac_iq_file_browse_clicked();

    void on_pb_ddr_dac_iq_file_send_clicked();

private:
    Ui::MainWindow *ui;
    QProgressDialog* transferProgress = nullptr;
    bool transferCanceled = false;
};
#endif // MAINWINDOW_H
