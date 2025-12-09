#ifndef FILEPROCESSING_H
#define FILEPROCESSING_H

#include "Utils.h"
#include <QWidget>
#include <packetforwarder.h>
namespace Ui {
class FileProcessing;
}

class FileProcessing : public QWidget
{
    Q_OBJECT

public:
    explicit FileProcessing(QWidget *parent = nullptr);
    ~FileProcessing();
    iface getSelectedDeviceType();
    PacketForwarder *relay;
    bool simulateCoordinateFileFits(const QString &filePath);

private slots:
    void on_PbFPTargetPostionSet_clicked();

    void on_PbFPFileBrowse_clicked();

    void on_PbFPCSVFileSend_clicked();

    void on_GRpBoxUserInput_clicked(bool checked);

    void on_GrpBoxFPRealTime_clicked(bool checked);

    void on_GrpBoxFPCsvFileSend_clicked(bool checked);

private slots:
    void onMeasurementUpdated(const RelayMeasurement &m);

    void on_PbFPTargetVelocitySet_clicked();

    void on_GRpBoxUserInput_clicked();

private:
    Ui::FileProcessing *ui;
};

#endif // FILEPROCESSING_H
