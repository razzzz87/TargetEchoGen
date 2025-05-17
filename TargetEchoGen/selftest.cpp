#include "selftest.h"
#include "ui_selftest.h"

SelfTest::SelfTest(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SelfTest)
{
    ui->setupUi(this);
}

SelfTest::~SelfTest()
{
    delete ui;
}
