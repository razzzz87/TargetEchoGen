#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <QWidget>
#include "qcustomplot.h"

namespace Ui {
class Spectrum;
}

class Spectrum : public QWidget
{
    Q_OBJECT

public:
    explicit Spectrum(QWidget *parent = nullptr);
    ~Spectrum();
    void makePlot();

private slots:
    void on_pd_show_hide_menu_clicked();

private:
    Ui::Spectrum *ui;
    QCustomPlot *customPlot;
};

#endif // SPECTRUM_H
