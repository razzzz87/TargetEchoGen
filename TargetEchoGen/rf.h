#ifndef RF_H
#define RF_H

#include <QWidget>

namespace Ui {
class RF;
}

class RF : public QWidget
{
    Q_OBJECT

public:
    explicit RF(QWidget *parent = nullptr);
    ~RF();

private slots:
    void on_PbDAC3TgrSetup_clicked();

    void on_PbDAC3TgrStop_clicked();

    void on_PbDAC3TgrStart_clicked();

    void on_PbDAC3FixedLevelSet_clicked();

private:
    Ui::RF *ui;
};

#endif // RF_H
