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

// Read data from CSV file
    QCustomPlot *customPlot = new QCustomPlot(this);
    QVector<double> wavelengths, intensities;
    QFile file("spectrum_data.csv");
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        QString line = in.readLine(); // Skip the header
        while (!in.atEnd())
        {
            line = in.readLine();
            QStringList fields = line.split(",");
            if (fields.size() == 2)
            {
                wavelengths.append(fields[0].toDouble());
                intensities.append(fields[1].toDouble());
            }
        }
        file.close();
    }
    else{
        qDebug() << "Failed to open file.";
        return ;
    }
    // Create graph and assign data to it
    customPlot->addGraph();
    customPlot->graph(0)->setData(wavelengths, intensities);
    customPlot->graph(0)->setPen(QPen(Qt::yellow)); // Set line color to yellow

    // Set background color to dark
    customPlot->setBackground(QBrush(Qt::black));
    customPlot->xAxis->setBasePen(QPen(Qt::white));
    customPlot->yAxis->setBasePen(QPen(Qt::white));
    customPlot->xAxis->setTickPen(QPen(Qt::white));
    customPlot->yAxis->setTickPen(QPen(Qt::white));
    customPlot->xAxis->setSubTickPen(QPen(Qt::white));
    customPlot->yAxis->setSubTickPen(QPen(Qt::white));
    customPlot->xAxis->setTickLabelColor(Qt::white);
    customPlot->yAxis->setTickLabelColor(Qt::white);
    customPlot->xAxis->setLabelColor(Qt::white);
    customPlot->yAxis->setLabelColor(Qt::white);

    // Give the axes some labels
    customPlot->xAxis->setLabel("Wavelength (nm)");
    customPlot->yAxis->setLabel("Intensity");

    // Set axes ranges, so we see all data
    customPlot->xAxis->setRange(400, 700);
    customPlot->yAxis->setRange(0, 1);

    // Enable dragging and zooming
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    // Customize the plot (optional)
    customPlot->resize(1900, 700);
    customPlot->replot();
}

Spectrum::~Spectrum()
{
    delete ui;
}
void Spectrum::makePlot(){

   /* QVector<double> x(100000), y(100000); // initialize with entries 0..100
    for (int i=0; i<100000; ++i)
    {
        x[i] = i/50.0 - 1; // x goes from -1 to 1
        y[i] = x[i]*x[i]; // let's plot a quadratic function
    }
    // create graph and assign data to it:
    customPlot->addGraph();
    customPlot->graph(0)->setData(x, y);

    // Apply colors to the plot
    //customPlot->graph(0)->setPen(QPen(lineColor)); // Set the line color
    //customPlot->graph(0)->setBrush(QBrush(fillColor.lighter(150))); // Set the fill color with some transparency
    //customPlot->setBackground(QBrush(backgroundColor)); // Set the background color

    // Change the color of the graph
    //customPlot->graph(0)->setPen(QPen(Qt::blue)); // Line color
    //customPlot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 50))); // Fill color with transparency

    // Set logarithmic scale for y-axis
    customPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    customPlot->yAxis->setTicker(logTicker);

    // Customize axes
    customPlot->xAxis->setLabel("x(log scale)");
    customPlot->yAxis->setLabel("y(log scale)");
    customPlot->xAxis->setRange(-500, 1000);
    customPlot->yAxis->setRange(-100, 1000); // Adjust range as needed

    customPlot->replot();
*/
}
