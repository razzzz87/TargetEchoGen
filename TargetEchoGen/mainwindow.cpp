#include "mainwindow.h"
#include "devicesetup.h"
#include "fileprocessing.h"
#include "filesender.h"
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

    EthPs01G = &UDP_PS1G_Con::getInstance();
    EthPl10G = &UDP_PL10G_Con::getInstance();
    EthPl01G = &UDP_PL1G_Con::getInstance();
    setupTransferAgent = new FileTransferAgent(this);
    ui->tabWidgetMainTab->addTab(new FileProcessing(),"File processing");
    ui->tabWidgetMainTab->addTab(deviceSetup,"Device setup");
    ui->tabWidgetMainTab->addTab(new SelfTest(),"Self Test");
    ui->tabWidgetMainTab->addTab(new Spectrum(),"Spectrum Analyzer");

    //ui->tabWidget_subTab->addTab(new RF(),"RF");

    ui->pb_refresh_button->setIconSize(QSize(ui->pb_refresh_button->width(), ui->pb_refresh_button->height()));
    ui->pb_conn_settings->setIconSize(QSize(ui->pb_conn_settings->width(), ui->pb_conn_settings->height()));
    ui->pb_conn_reset->setIconSize(QSize(ui->pb_conn_reset->width(), ui->pb_conn_reset->height()));
    ui->label_device_temp_dig_val->setText(tr("%1 °C").arg(100));
    ui->label_device_temp_ana_val->setText(tr("%1 °C").arg(100));
    load_files();


    // 🔄 Initialize QProgressDialog for percentage-based progress tracking
    transferProgress = new QProgressDialog("Preparing file transfer...", "Cancel", 0, 100, this);
    transferProgress->setWindowModality(Qt::WindowModal);
    transferProgress->setWindowTitle("File Transfer Progress");
    transferProgress->setAutoClose(false);     // Keep open until transfer finishes
    transferProgress->setAutoReset(false);     // Manual control over reset
    transferProgress->setMinimumDuration(1000); // Avoid premature popup (1s)
    transferProgress->reset();                 // Clear stale values
    transferProgress->hide();                  // Hide until file transfer starts

    // Connect cancel behavior just once
    connect(transferProgress, &QProgressDialog::canceled, this, [=]() {
        transferCanceled = true;
        setupTransferAgent->abortFileWrite(true);
        LOG_TO_FILE("User canceled the file transfer.");
    });
    connect(setupTransferAgent, &FileTransferAgent::progressUpdated,this, &MainWindow::updateTransferProgress);
    connect(setupTransferAgent, &FileTransferAgent::close_progress_pop,this, &MainWindow::close_Progress_pop);
}

MainWindow::~MainWindow()
{
    delete file_processing;
    delete device_setup;
    delete ui;
}

void MainWindow::onTimeout()
{

}

void MainWindow::load_files()
{
        QDir directory("C:\\Users\\razzz\\OneDrive\\Documents\\TargetEchoGen\\data");
        QStringList files = directory.entryList(QDir::Files);

        //ui->tableWidget_playback->setRowCount(files.size());
        //ui->tableWidget_playback->setColumnCount(3);
        //ui->tableWidget_playback->setHorizontalHeaderLabels(QStringList() << "File Name" << "Date Time" << "Size(MB)");
        //ui->tableWidget_playback->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        //ui->tableWidget_playback->setSelectionBehavior(QAbstractItemView::SelectRows);
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

           // ui->tableWidget_playback->setItem(row, 0, fileNameItem);
           // ui->tableWidget_playback->setItem(row, 1, modifiedDateItem);
           // ui->tableWidget_playback->setItem(row, 2, fileSizeItem);
        }
}

void MainWindow::close_Progress_pop(void){

    transferProgress->reset();
    transferProgress->close();
}
void MainWindow::updateTransferProgress(qint64 percentage){

    transferProgress->setValue(qBound(0, percentage, 100));
}

void MainWindow::on_pb_conn_settings_clicked()
{
    conn->show();
}


void MainWindow::on_pb_refresh_button_clicked()
{

}


void MainWindow::on_pb_ddr_dac_iq_file_browse_clicked()
{
    LOG_TO_FILE(":Entry==>");
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open File",
                                                    ".",
                                                    "Text Files (*.bin);;All Files (*)");
    if (!fileName.isEmpty())
    {
        qDebug() << "FileName" << fileName;
        LOG_TO_FILE("Filename:%s",fileName.toStdString().c_str());
        //ui->lineEdit_ddr_dac_iq_file_name_path->setText(fileName);
    }
    LOG_TO_FILE(":Exit==>\n");
}


void MainWindow::on_pb_ddr_dac_iq_file_send_clicked()
{
    QString selectedFile = ui->lineEdit_ddr_dac_iq_file_name_path->text();
    if (selectedFile.isEmpty()) return;

    QFile file(selectedFile);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_TO_FILE("Failed to open file: %s", selectedFile.toUtf8().constData());
        return;
    }
    int totalSize = file.size();
    file.close();

    LOG_TO_FILE("File selected (%d bytes): %s", totalSize, selectedFile.toUtf8().constData());
    setupTransferAgent->setupDevice(EthPs01G);
    setupTransferAgent->configure(EthPs01G->remote_ip, EthPs01G->remote_port, selectedFile, totalSize,HostToTarget);
    setupTransferAgent->start();
    transferProgress->setRange(0, 100);
    transferProgress->show();
}

