#include "rf.h"
#include "ui_rf.h"

RF::RF(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RF)
{
    ui->setupUi(this);
}

RF::~RF()
{
    delete ui;
}
