#include "mainwindow.h"
#include "devicesetup.h"
#include "fileprocessing.h"
#include "rf.h"
#include "selftest.h"
#include "spectrum.h"
#include "ui_mainwindow.h"
#include <QTableWidgetItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDir>
#include <QFileInfoList>
#include <QHostAddress>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    conn = new ConnectionType(this);
    DeviceSetup *deviceSetup = new DeviceSetup();

    ui->tabWidgetMainTab->addTab(new FileProcessing(),"File processing");
    ui->tabWidgetMainTab->addTab(deviceSetup,"Device setup");
    ui->tabWidgetMainTab->addTab(new SelfTest(),"Self Test");
    ui->tabWidgetMainTab->addTab(new Spectrum(),"Spectrum Analyzer");


    ui->tabWidget_subTab->addTab(new RF(),"RF");

    ui->pushButton_refresh_button->setIconSize(QSize(ui->pushButton_refresh_button->width(), ui->pushButton_refresh_button->height()));
    ui->pushButton_conn_settings->setIconSize(QSize(ui->pushButton_conn_settings->width(), ui->pushButton_conn_settings->height()));
    ui->pushButton_con_reset->setIconSize(QSize(ui->pushButton_con_reset->width(), ui->pushButton_con_reset->height()));
    ui->label_device_temp_dig_val->setText(tr("%1 °C").arg(100));
    ui->label_device_temp_ana_val->setText(tr("%1 °C").arg(100));
    load_files();
}

MainWindow::~MainWindow()
{
    delete file_processing;
    delete device_setup;
    delete ui;
}

void MainWindow::on_pushButton_conn_settings_clicked()
{
    conn->show();
}

void MainWindow::load_files()
{
        QDir directory("C:\\Users\\razzz\\OneDrive\\Documents\\TargetEchoGen\\data");
        QStringList files = directory.entryList(QDir::Files);

        ui->tableWidget_playback->setRowCount(files.size());
        ui->tableWidget_playback->setColumnCount(3);
        ui->tableWidget_playback->setHorizontalHeaderLabels(QStringList() << "File Name" << "Date Time" << "Size(MB)");
        ui->tableWidget_playback->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableWidget_playback->setSelectionBehavior(QAbstractItemView::SelectRows);
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

            ui->tableWidget_playback->setItem(row, 0, fileNameItem);
            ui->tableWidget_playback->setItem(row, 1, modifiedDateItem);
            ui->tableWidget_playback->setItem(row, 2, fileSizeItem);
        }
}
void MainWindow::on_pushButton_ddr_if_file_browse_clicked()
{
    LOG_TO_FILE(":Entry==>");
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open File",
                                                    ".",
                                                    "Text Files (*.txt);;All Files (*)");

    if (!fileName.isEmpty()){
        qDebug() << "FileName" << fileName;
        LOG_TO_FILE("Filename:%s",fileName.toStdString().c_str());
        ui->lineEdit_ddr_if_file_path->setText(fileName);
    }
    LOG_TO_FILE(":Exit==>\n");
}

void MainWindow::on_pushButton_ddr_if_amplitude_file_browse_clicked()
{
    LOG_TO_FILE(":Entry==>");
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open File",
                                                    ".",
                                                    "Text Files (*.txt);;All Files (*)");

    if (!fileName.isEmpty()){
        qDebug() << "FileName" << fileName;
        LOG_TO_FILE("Filename:%s",fileName.toStdString().c_str());
        ui->lineEdit_ddr_if_file_path->setText(fileName);
    }
    LOG_TO_FILE(":Exit==>\n");
}


void MainWindow::on_pushButton__ddr_if_amplitude_file_send_clicked()
{
     LOG_TO_FILE(":Entry==>");
    if(ui->lineEdit_ddr_if_file_path->text().isEmpty()){
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Please select file\n");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        LOG_TO_FILE("Return after showing msgbox\n");
        return;
    }
    LOG_TO_FILE("Continue to send file\n");
    LOG_TO_FILE(":Exit==>\n");
}

void MainWindow::on_pushButton_ddr_if_dac1_send_clicked()
{
    LOG_TO_FILE(":Entry==>");
    if(ui->lineEdit_ddr_if_file_path->text().isEmpty()){
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Please select file\n");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        LOG_TO_FILE("Return after showing msgbox\n");
        return;
    }
    qDebug() << "Continue to send file\n";
    LOG_TO_FILE("Continue to send file\n");
    LOG_TO_FILE(":Exit==>\n");
}


void MainWindow::on_pushButton_ddr_lx_file_browse_clicked()
{
    LOG_TO_FILE(":Entry==>");
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open File",
                                                    ".",
                                                    "Text Files (*.txt);;All Files (*)");

    if (!fileName.isEmpty()){
        qDebug() << "FileName" << fileName;
        LOG_TO_FILE("Filename:%s",fileName.toStdString().c_str());
        ui->lineEdit_ddr_lx_file_path->setText(fileName);
    }
    LOG_TO_FILE(":Exit==>\n");
}

