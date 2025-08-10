#include "spectrum.h"
#include "ui_spectrum.h"
#include <QVector>
#include <cmath>
#include <complex>
#include <vector>
#include <QApplication>
#include <QVector>
#include <cmath>
#include <vector>

#include <QApplication>
#include <QVector>
#include <cmath>
#include <QSizePolicy>
#include <qwt_plot_grid.h>

Spectrum::Spectrum(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Spectrum)
{
    ui->setupUi(this);
    X_graphPlot = new QwtPlot(ui->frame_plot_area);
    X_graphPlot->setStyleSheet("background-color: rgb(0, 0, 0);color: rgb(255, 170, 0);border:none");
    X_graphPlot->setAxisScale(QwtPlot::yLeft,-150,10,10);
    X_graphPlot->setAxisTitle(QwtPlot::yLeft,"dBm");
    X_graphPlot->setAxisTitle(QwtPlot::xBottom,"MHz");

    picker = new QwtPlotPicker(X_graphPlot->xBottom, X_graphPlot->yLeft, QwtPicker::NoRubberBand, QwtPicker::AlwaysOn, X_graphPlot->canvas());
    picker->setRubberBandPen( QColor( Qt::red ) );
    picker->setTrackerPen( QColor( Qt::red ) );

    QHBoxLayout *obj= new QHBoxLayout(this);
    ui->frame_plot_area->setLayout(obj);
    obj->addWidget(X_graphPlot);
    obj->addWidget(ui->frame_plot_area);
    QFont font;
    font.setBold(true);
    font.setFamily("TimesNewRoman");
    font.setPointSizeF(16);
    picker->setTrackerFont(font);

    pickerMachine = new QwtPickerDragPointMachine();

    picker->setStateMachine(pickerMachine);
    connect(picker,SIGNAL(moved(QPoint)),this,SLOT(plotPicker(QPoint)));

    FilePlay =false;

    this->setStyleSheet("border:none;");
    QwtPlotGrid *grid = new QwtPlotGrid;
    /// Set style sheet of grid /////
    grid->setPen(Qt::gray,0,Qt::DashDotLine);
    grid->attach(X_graphPlot);
    adcData_Point = (char *)malloc(0x100000);
    curve_Y= new QwtPlotCurve() ;
    curve_Y->attach(X_graphPlot);
    X_graphPlot->show();
    m_ObjMaxCurve    = new QwtPlotCurve();

    //obj_thread = new MAPI_Thread() ;
    plotTimer = new QTimer(this);
    //connect(obj_thread,SIGNAL(finished()),this,SLOT(thread_Finished()));
    connect(plotTimer,SIGNAL(timeout()),this,SLOT(on_plotTimer_TimeOut()));
    in = (double*) fftw_malloc(sizeof(double) * NUM_POINTS);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * NUM_POINTS);
    ui->strmnStop_radioButton->setChecked(true);
    ui->autoRefreshOff_radioButton->setChecked(true);
    // ui->pushButton_4->setEnabled(false);
    sample_count=0;
    maxHold= false;
    windowSize=2048;
    ui->DataSize_comboBox->setCurrentIndex(1);
    ui->DDC_DataradioButton->setChecked(true);
    ui->IQInterleved_radioButton->setChecked(true);
    ui->frame_plot_control->hide();
    this->setMouseTracking(true);
    ui->frame_plot_control->hide();
    ui->pb_show_hide_menu->setText("Show Menu");
}

Spectrum::~Spectrum()
{
    delete ui;
}

void Spectrum::FFT()
{

    double *t = new double[ui->windowSize_lineEdit->text().toInt()];
    //fread()
    for (int i=0; i<= ui->windowSize_lineEdit->text().toInt();i++)
    {
        t[i]=i*1/ui->lineEdit->text().toInt();

        in[i] =0.7 *sin(2*M_PI*f*t[i]);// generate sine waveform
        double multiplier = 0.5 * (1 - cos(2*M_PI*i/(windowSize-1)));//Hanning Window
        in[i] = multiplier * in[i];
    }

    for (int i=0; i<= ((windowSize/2)-1);i++)
    {ff[i]=Fs*i/windowSize;
    }
    plan_forward = fftw_plan_dft_r2c_1d ( windowSize, in, out, FFTW_ESTIMATE );

    fftw_execute ( plan_forward );

    double *v = new double[windowSize];

    for (int i = 0; i<= ((windowSize/2)-1); i++)
    {
        v[i]=(20*log(sqrt(out[i][0]*out[i][0]+ out[i][1]*out[i][1])))/NUM_POINTS;  //Here   I  have calculated the y axis of the spectrum in dB
    }
    QwtPlotCurve *curve = new QwtPlotCurve() ;
    curve->setSamples(ff,v,275);
    curve->setPen(Qt::red,1.5,Qt::DotLine);

    curve->attach(X_graphPlot);
    X_graphPlot->replot();
    X_graphPlot->show();

    fftw_destroy_plan ( plan_forward );
    fftw_free ( in );
    fftw_free ( out );

}

void Spectrum::FFT_Plot(int windowSize, fftw_complex* signal, fftw_complex* outBuffer)
{
    try {
        std::vector<double> freq(windowSize);
        for (int i = 0; i < windowSize; ++i)
            freq[i] = Fs * i / (windowSize - 1);

        fftw_plan plan_forward = fftw_plan_dft_1d(windowSize, signal, outBuffer, FFTW_FORWARD, FFTW_ESTIMATE);
        fftw_execute(plan_forward);

        std::vector<double> magnitude(windowSize);
        for (int i = 0; i < windowSize; ++i)
            magnitude[i] = std::hypot(outBuffer[i][0], outBuffer[i][1]);

        if (maxHold) {
            int n2 = windowSize / 2;
            for (int i = 0; i < n2; ++i)
                std::swap(magnitude[i], magnitude[i + n2]);

            double xval = -Fs / 2;
            for (int i = 0; i < windowSize; ++i) {
                freq[i] = xval;
                xval += Fs / windowSize;
            }
        }

        double maxAmplitude = *std::max_element(magnitude.begin(), magnitude.end());
        int maxIndex = std::distance(magnitude.begin(), std::max_element(magnitude.begin(), magnitude.end()));

        if (ui->enableWeight_checkBox->isChecked())
            maxAmplitude = ui->enableWeight_lineEdit->text().toLongLong();

        std::vector<float> dataspectrum(windowSize);
        std::vector<double> dB(windowSize);
        for (int i = 0; i < windowSize; ++i) {
            double val = magnitude[i] / maxAmplitude;
            dB[i] = 20 * std::log10(val);
            dataspectrum[i] = static_cast<float>(val);
        }

        double max_db = dB[maxIndex];
        qDebug() << "V at max index" << max_db;

        if (ui->m_CBMaxHold->isChecked()) {
            if (!m_bMaxHoldEnable)
                m_vMaxHoldBuffer = new double[windowSize];

            for (int i = 0; i < windowSize; ++i) {
                double val = magnitude[i] / maxAmplitude;
                dB[i] = 20 * std::log10(val);
                if (!m_bMaxHoldEnable || m_vMaxHoldBuffer[i] < dB[i])
                    m_vMaxHoldBuffer[i] = dB[i];
            }

            m_bMaxHoldEnable = true;
            int plotSize = ui->IOnly_radioButton->isChecked() ? windowSize / 2 : windowSize;
            m_ObjMaxCurve->setSamples(freq.data(), m_vMaxHoldBuffer, plotSize);
            m_ObjMaxCurve->setPen(Qt::green, 1.5, Qt::SolidLine);
            m_ObjMaxCurve->attach(X_graphPlot);
        }

        int plotSize = ui->IOnly_radioButton->isChecked() ? windowSize / 2 : windowSize;
        curve_Y->setSamples(freq.data(), dB.data(), plotSize);
        curve_Y->setPen(Qt::yellow, 1.5, Qt::SolidLine);
        X_graphPlot->replot();

        double frequency = maxIndex * Fs / windowSize;
        qDebug() << "Max Db" << max_db << frequency << windowSize << maxIndex << Fs;
        ui->frequency_label->setText(QString::number(frequency));
        ui->frequency_label2->setText(QString::number(max_db));

        if (ui->m_CBSpectrumdata->isChecked() && spectrogramWidget->isVisible()) {
            // emit bufferFilled(dataspectrum.data(), windowSize, maxAmplitude);
        }

        fftw_destroy_plan(plan_forward);
    }
    catch (const std::exception& e) {
        qDebug() << "Exception occurred:" << e.what();
    }
}

void Spectrum::FFT_Plot(int windowSize, double* sample, fftw_complex* outBuffer)
{
    // Frequency axis
    std::vector<double> freq(windowSize / 2);
    for (int i = 0; i < windowSize / 2; ++i)
        freq[i] = Fs * i / windowSize;

    // Create FFT plan
    fftw_plan plan_forward = fftw_plan_dft_r2c_1d(windowSize, sample, outBuffer, FFTW_ESTIMATE);
    fftw_execute(plan_forward);

    // Magnitude spectrum
    std::vector<double> magnitude(windowSize / 2);
    for (int i = 0; i < windowSize / 2; ++i)
        magnitude[i] = std::hypot(outBuffer[i][0], outBuffer[i][1]);

    // Plotting
    curve_Y->setSamples(freq.data(), magnitude.data(), windowSize / 2);
    curve_Y->setPen(Qt::yellow, 1.5, Qt::SolidLine);
    curve_Y->attach(X_graphPlot);
    X_graphPlot->replot();
    X_graphPlot->show();

    // Cleanup
    fftw_destroy_plan(plan_forward);
}

void Spectrum::on_pb_play_from_file_clicked()
{
    if(ui->strmnStrt_radioButton->isChecked()==false)
    {
        QMessageBox::critical(this,"Warning","Please select Start option before Playing recorded Data from FIle",QMessageBox::Ok);
        return;
    }
    if(!FilePlay)
    {
        QString filter = "File Description (*.bin;*.hex;)";
        QString fileName = QFileDialog::getOpenFileName(this, "Select a file...",QDir::currentPath(), filter);
        if(fileName.isEmpty())
        {
            QMessageBox::information(this,"Information","Please select File to Play recorded data",QMessageBox::Ok);
            return;
        }
        playBack_Counter=0;
        if(!(filePlayBack_fp= fopen(fileName.toStdString().c_str(),"rb")))
        {
            return;
        }
        fseek(filePlayBack_fp, 0, SEEK_END);
        dwFileSize = ftell(filePlayBack_fp);
        fseek(filePlayBack_fp, 0, SEEK_SET);
        playBack_Timer = new QTimer(this);
        playBack_Timer->setInterval(ui->refrestRate_lineEdit->text().toInt());
        connect(playBack_Timer,SIGNAL(timeout()),this,SLOT(playFile()));
        playBack_Timer->start();
        ui->pb_play_from_file->setText("Stop Playing  Recorded Data");
        FilePlay=true;
        ui->strmnStrt_radioButton->setDisabled(true);
        ui->strmnStop_radioButton->setDisabled(true);
    }
    else
    {
        playBack_Timer->stop();
        fclose(filePlayBack_fp);
        playBack_Counter=0;
        ui->progressBar->setValue(0);
        ui->pb_play_from_file->setText("Start Playing Recorded Data");
        ui->strmnStrt_radioButton->setDisabled(false);
        ui->strmnStop_radioButton->setDisabled(false);
        ui->IOnly_radioButton->setDisabled(false);
        ui->IQInterleved_radioButton->setDisabled(false);
        ui->raw_radioButton->setDisabled(false);
        ui->DDC_DataradioButton->setDisabled(false);
        FilePlay= false;
    }
}


void Spectrum::on_pb_show_hide_menu_clicked()
{
    if(ui->pb_show_hide_menu->text() == "Show Menu"){
        ui->frame_plot_control->show();
        ui->pb_show_hide_menu->setText("Hide Menu");
    }
    else{
        ui->frame_plot_control->hide();
        ui->pb_show_hide_menu->setText("Show Menu");
    }
}

