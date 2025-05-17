#ifndef FILEPROCESSING_H
#define FILEPROCESSING_H

#include <QWidget>

namespace Ui {
class FileProcessing;
}

class FileProcessing : public QWidget
{
    Q_OBJECT

public:
    explicit FileProcessing(QWidget *parent = nullptr);
    ~FileProcessing();

private:
    Ui::FileProcessing *ui;
};

#endif // FILEPROCESSING_H
