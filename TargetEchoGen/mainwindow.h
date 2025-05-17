#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "connectiontype.h"
#include <QMainWindow>
#include <QGroupBox>
#include <udpcon.h>

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
    void load_files();
private slots:
    void on_pushButton_conn_settings_clicked();

    void on_pushButton_ddr_if_file_browse_clicked();

    void on_pushButton_ddr_if_amplitude_file_browse_clicked();

    void on_pushButton__ddr_if_amplitude_file_send_clicked();

    void on_pushButton_ddr_if_dac1_send_clicked();

    void on_pushButton_ddr_lx_file_browse_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
