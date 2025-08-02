#include "spectrum.h"
#include "ui_spectrum.h"
#include "qcustomplot.h"
#include <QVector>
#include <cmath>
#include <complex>
#include <vector>
#include <QApplication>
#include <QVector>
#include <cmath>
#include <complex>
#include <vector>

#include <QApplication>
#include <QVector>
#include <cmath>
#include <complex>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <QSizePolicy>


using namespace std;
// Function to perform FFT
void fft(const std::vector<std::complex<double>>& in, std::vector<std::complex<double>>& out) {
    size_t N = in.size();
    if (N <= 1) {
        out = in;
        return;
    }

    std::vector<std::complex<double>> even(N / 2), odd(N / 2);
    for (size_t i = 0; i < N / 2; ++i) {
        even[i] = in[i * 2];
        odd[i] = in[i * 2 + 1];
    }

    std::vector<std::complex<double>> fftEven(N / 2), fftOdd(N / 2);
    fft(even, fftEven);
    fft(odd, fftOdd);

    out.resize(N);
    for (size_t k = 0; k < N / 2; ++k) {
        std::complex<double> t = std::polar(1.0, -2 * M_PI * k / N) * fftOdd[k];
        out[k] = fftEven[k] + t;
        out[k + N / 2] = fftEven[k] - t;
    }
}

Spectrum::Spectrum(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Spectrum)
{
    ui->setupUi(this);
    //ui->frame_plot_control_area->hide();
    customPlot = new QCustomPlot();

    makePlot();
}

Spectrum::~Spectrum()
{
    delete ui;
}
void Spectrum::makePlot() {
    // Create the plot and attach it to the UI frame
    customPlot->setParent(ui->frame_plot_area);
    //customPlot->setFixedSize(ui->frame_plot_area->width(), ui->frame_plot_area->height());
    // Ensure layout exists
    if (!ui->frame_plot_area->layout()) {
        QVBoxLayout* layout = new QVBoxLayout(ui->frame_plot_area);
        layout->setContentsMargins(0, 0, 0, 0); // Optional: remove spacing
        ui->frame_plot_area->setLayout(layout);
    }
    // Add the plot to the layout
    ui->frame_plot_area->layout()->addWidget(customPlot);

    qDebug () << "width" << ui->frame_plot_area->width() << "height" << ui->frame_plot_area->height();
    // Set black background
    customPlot->setBackground(QColor(25, 25, 25));

    // Simulated spectrum data
    QVector<double> x(1024), y(1024);
    for (int i = 0; i < 1024; ++i) {
        x[i] = i;  // Frequency from 0 to 1023 MHz
        double noise = (rand() % 20 - 10) * 0.5;
        y[i] = -150 + 80 * exp(-0.0001 * pow(x[i] - 500, 2)) + noise;  // Peak near 500 MHz
    }

    // Add graph
    customPlot->addGraph();
    customPlot->graph(0)->setData(x, y);
    customPlot->graph(0)->setPen(QPen(QColor(0, 255, 0), 2));  // Bright green
    customPlot->graph(0)->setBrush(QBrush(QColor(0, 255, 0, 30)));  // Subtle fill

    // Axis styling
    auto axisStyle = [](QCPAxis* axis) {
        axis->setBasePen(QPen(QColor(255, 165, 0)));
        axis->setTickPen(QPen(QColor(255, 165, 0)));
        axis->setSubTickPen(QPen(QColor(255, 165, 0)));
        axis->setTickLabelColor(QColor(255, 165, 0));
        axis->setLabelColor(QColor(255, 165, 0));
        axis->setTickLabelFont(QFont("Courier", 8));
    };
    axisStyle(customPlot->xAxis);
    axisStyle(customPlot->yAxis);

    // Axis labels and fixed ranges
    customPlot->xAxis->setLabel("Frequency (MHz)");
    customPlot->xAxis->setLabelColor(QColor(255, 165, 0));
    customPlot->yAxis->setLabel("Amplitude (dBm)");
    customPlot->yAxis->setLabelColor(QColor(255, 165, 0));
    customPlot->xAxis->setRange(0, 1000);
    customPlot->yAxis->setRange(-150, 100);  // ✅ Only this is needed
    customPlot->yAxis->setRange(-150, 10);

    customPlot->yAxis->setSubTickLengthIn(0);   // 5 pixels inward
    customPlot->yAxis->setSubTickLengthOut(5);  // 0 pixels ou

    //set tick steps,
    QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
    fixedTicker->setTickStep(10);
    fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssNone);
    customPlot->yAxis->setTicker(fixedTicker);

    // Adjust margins to ensure full visibility
    QMargins margins(50, 20, 20, 40);  // left, top, right, bottom
    customPlot->axisRect()->setMargins(margins);

    // Grid styling
    QPen gridPen(QColor(255, 165, 0), 1, Qt::DashDotLine);
    customPlot->xAxis->grid()->setPen(gridPen);
    customPlot->yAxis->grid()->setPen(gridPen);
    customPlot->xAxis->grid()->setVisible(true);
    customPlot->yAxis->grid()->setVisible(true);
    // Final render
    customPlot->replot();
}

void Spectrum::on_pd_show_hide_menu_clicked()
{
    if(ui->pd_show_hide_menu->text() == "Show Menu"){
        ui->frame_plot_control->show();
        ui->pd_show_hide_menu->setText("Hide Menu");

    }
    else{
        ui->frame_plot_control->hide();
        ui->pd_show_hide_menu->setText("Show Menu");
    }
    makePlot();
}

