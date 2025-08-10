#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <QWidget>
#include "qcustomplot.h"
#include <fftw3.h>
#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot.h>
#include <qwt_matrix_raster_data.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_marker.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_point_data.h>
#include "qspectrogram.h"
#include "spectrogram.h"

#define NUM_POINTS 2048  //Number of points acquired inside the window
namespace Ui {
class Spectrum;
}

class Spectrum : public QWidget
{
    Q_OBJECT

public:
    QTimer *plotTimer;
    QTimer *playBack_Timer;
    double *data;
    unsigned long long dwFileSize;
    char* adcData_Point;
    long readDataFromFpGA();
    double Fs;
    QwtPlot* X_graphPlot;
    QwtPlotCurve *curve_Y ;
    QwtPlotPicker *picker;
    int windowSize;
    int sample_count;
    unsigned long long playBack_Counter;// vaiable use to see how many bytes is processed
    unsigned int RAWMode_windowSize;
    bool maxHold;
    std::vector<double> numbers;
    QList <double> fft_data;
    FILE *filePlayBack_fp ;
    bool FilePlay;
    QwtPickerMachine* pickerMachine;
    QwtPlotCurve *m_ObjMaxCurve ;
    bool m_bMaxHoldEnable;
    double *m_vMaxHoldBuffer;
    double y;
    //double Fs=200;//sampling frequency
    //double dF=Fs/N;
    //double  T=1/Fs;//sample time
    double f=50;//frequencyr
    double *in;
    fftw_complex *out;
    fftw_complex *out2;
    double t[NUM_POINTS];//time vector
    double ff[NUM_POINTS];
    fftw_plan plan_forward;
    fftw_plan plan_forward2;
    Spectrogram *spectrogram;
    QSpectrogram *spectrogramWidget;

    void hide();

    explicit Spectrum(QWidget *parent = nullptr);
    ~Spectrum();

    void FFT();
    void FFT_Plot(int windowSize, double *sample, fftw_complex *outBuffer);
    void FFT_Plot(int windowSize, fftw_complex *signal, fftw_complex *outBuffer);

    void makePlot();

private slots:
    void on_pb_play_from_file_clicked();
    void on_pb_show_hide_menu_clicked();
private:
    Ui::Spectrum *ui;
    QCustomPlot *customPlot;
};

#endif // SPECTRUM_H
