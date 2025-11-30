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

private slots:
    void on_PbFPTargetPostionSet_clicked();

    void on_PbFPFileBrowse_clicked();

    void on_PbFPCSVFileSend_clicked();

private:
    Ui::FileProcessing *ui;
};

#endif // FILEPROCESSING_H
