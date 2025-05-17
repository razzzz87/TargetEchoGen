#include "fileprocessing.h"
#include "ui_fileprocessing.h"

FileProcessing::FileProcessing(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FileProcessing)
{
    ui->setupUi(this);
}

FileProcessing::~FileProcessing()
{
    delete ui;
}
