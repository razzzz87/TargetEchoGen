#ifndef SELFTEST_H
#define SELFTEST_H

#include <QWidget>

namespace Ui {
class SelfTest;
}

class SelfTest : public QWidget
{
    Q_OBJECT

public:
    explicit SelfTest(QWidget *parent = nullptr);
    ~SelfTest();

private:
    Ui::SelfTest *ui;
};

#endif // SELFTEST_H
