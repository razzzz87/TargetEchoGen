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

private:
    Ui::RF *ui;
};

#endif // RF_H
