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
#include <qwt_plot_grid.h>
#include <log.h>
#include <QTimer>
#include <QtGlobal>
#include <QRandomGenerator>
using namespace std;
Spectrum::Spectrum(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Spectrum)
{
    ui->setupUi(this);
    N = NUM_POINT;
    frq = FRQ;

    ui->DDC_DataradioButton->setStyleSheet("rgb(255,255,255)");
    ui->IQInterleved_radioButton->setStyleSheet("rgb(255,255,255)");
    ui->IOnly_radioButton->setStyleSheet("rgb(255,255,255)");
    ui->LblChannel->setStyleSheet("rgb(255,255,255)");
    ui->LblDataSize->setStyleSheet("rgb(255,255,255)");
    ui->LblFs->setStyleSheet("rgb(255,255,255)");
    ui->LblWindowSize->setStyleSheet("rgb(255,255,255)");
    ui->enableWeight_checkBox->setStyleSheet("rgb(255,255,255)");
    ui->ChkBoxFFtShift->setStyleSheet("rgb(255,255,255)");
    ui->m_CBMaxHold->setStyleSheet("rgb(255,255,255)");
    ui->LblRefreshRate->setStyleSheet("rgb(255,255,255)");
    ui->strmnStrt_radioButton->setStyleSheet("rgb(255,255,255)");
    ui->strmnStop_radioButton->setStyleSheet("rgb(255,255,255)");
    ui->autoRefreshOn_radioButton->setStyleSheet("rgb(255,255,255)");
    ui->autoRefreshOff_radioButton->setStyleSheet("rgb(255,255,255)");
    ui->byteReverse_checkBox->setStyleSheet("rgb(255,255,255)");
    ui->m_CBSpectrumdata->setStyleSheet("rgb(255,255,255)");



    ui->frequency_label->setStyleSheet("color: rgb(235, 13, 43); font: 24pt \"MS UI Gothic\";");
    ui->frequency_label_db->setStyleSheet("color: rgb(235, 13, 43); font: 24pt \"MS UI Gothic\";");
    ui->label_mhz->setStyleSheet("color: rgb(255, 170, 0);\nfont: 28pt \"MS UI Gothic\";");
    ui->label_db->setStyleSheet("color: rgb(255, 170, 0);\nfont: 28pt \"MS UI Gothic\";");

    X_graphPlot = new QwtPlot(ui->spectrum_plot_frame);
    X_graphPlot->setStyleSheet("background-color: rgb(0, 0, 0);color: rgb(255, 170, 0);border:none");
    X_graphPlot->setAxisScale(QwtPlot::yLeft,-150,10,10);
    X_graphPlot->setAxisTitle(QwtPlot::yLeft,"dBm");
    X_graphPlot->setAxisTitle(QwtPlot::xBottom,"MHz");
    X_graphPlot->plotLayout()->setCanvasMargin(10,QwtPlot::xBottom);

    picker = new QwtPlotPicker(X_graphPlot->xBottom, X_graphPlot->yLeft, QwtPicker::NoRubberBand, QwtPicker::AlwaysOn, X_graphPlot->canvas());
    picker->setRubberBandPen( QColor( Qt::red ) );
    picker->setTrackerPen( QColor( Qt::red ) );

    QHBoxLayout *obj= new QHBoxLayout(this);
    ui->spectrum_plot_frame->setLayout(obj);
    obj->addWidget(X_graphPlot);
    obj->addWidget(ui->frame_plot_control);

    QFont font;
    font.setBold(true);
    font.setFamily("TimesNewRoman");
    font.setPointSizeF(16);
    picker->setTrackerFont(font);

    pickerMachine = new QwtPickerDragPointMachine();

    picker->setStateMachine(pickerMachine);
    connect(picker,SIGNAL(moved(QPoint)),this,SLOT(plotPicker(QPoint)));

    FilePlay =false;

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

    //connect(obj_thread,SIGNAL(sThreadFinish()),this,SLOT(thread_Finished()));
    connect(plotTimer,SIGNAL(timeout()),this,SLOT(on_plotTimer_TimeOut()));
    //connect(obj_thread, SIGNAL(readWrite_Status(qint64)), this, SLOT(UpdateProgressBAR(qint64)));


    in = (double*) fftw_malloc(sizeof(double) * N);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    ui->strmnStop_radioButton->setChecked(true);
    ui->autoRefreshOff_radioButton->setChecked(true);

    sample_count=0;
    maxHold= false;
    windowSize=2048;
    ui->DataSize_comboBox->setCurrentIndex(1);
    ui->DDC_DataradioButton->setChecked(true);
    ui->IQInterleved_radioButton->setChecked(true);
    ui->frame_plot_control->hide();
    ui->pb_hide_show_menu->setText("Show Menu");
    m_vMaxHoldBuffer=NULL;

    // Disable auto-exclusive behavior
    ui->strmnStrt_radioButton->setAutoExclusive(false);
    ui->strmnStop_radioButton->setAutoExclusive(false);
    //ui->raw_radioButton->setAutoExclusive(false);
    ui->DDC_DataradioButton->setAutoExclusive(false);
    ui->IQInterleved_radioButton->setAutoExclusive(false);
    ui->IOnly_radioButton->setAutoExclusive(false);

    //ui->raw_radioButton->setChecked(false);
    ui->DDC_DataradioButton->setChecked(false);
    ui->IQInterleved_radioButton->setChecked(false);
    ui->IOnly_radioButton->setChecked(false);

    setupTransferAgent = new FileTransferAgent();
    connect(setupTransferAgent, &FileTransferAgent::transferComplete, this, &Spectrum::chunkReadCompleted);
}

Spectrum::~Spectrum()
{
    delete ui;
}
void Spectrum::chunkReadCompleted()
{
    LOG_INFO("Transfer completed");
}

void Spectrum::resizeEvent(QResizeEvent *)
{

}

void Spectrum::fftshift(void *data, int N, size_t elementSize) {
    int half = N / 2;
    int offset = (N % 2 == 0) ? half : half + 1; // handle odd/even case

    // Allocate temp buffer for shifting
    void *temp = malloc(offset * elementSize);
    if (!temp) {
        fprintf(stderr, "Memory allocation failed!\n");
        return;
    }

    // Copy first part into temp
    memcpy(temp, data, offset * elementSize);

    // Shift the second part to the beginning
    memmove(data, (char*)data + offset * elementSize, (N - offset) * elementSize);

    // Copy temp (first part) to the end
    memcpy((char*)data + (N - offset) * elementSize, temp, offset * elementSize);

    free(temp);
}


void Spectrum::plotPicker(QPoint actualMousePosition)
{
    int x = curve_Y->closestPoint(actualMousePosition,NULL)*Fs/windowSize;
    QPointF Position = curve_Y->sample(x);
    QwtPlotMarker Marker;
    Marker.setValue(Position);
    X_graphPlot->replot();
}

void Spectrum::thread_Finished()
{
    delete stat_Message;
    //obj_thread->StopThread = false;
}

void Spectrum::on_plotTimer_TimeOut()
{
    on_pb_plot_clicked();
    on_pb_play_snap_shot_clicked();
    //LOG_INFO("This should hit once");
}

void Spectrum::mouseMoveEvent(QMouseEvent *e)
{
    if(ui->pb_hide_show_menu->text()=="Show Menu")
    {
        float XRnage = ui->spectrum_plot_frame->width()-ui->spectrum_plot_frame->width();
        if(e->pos().x() >= XRnage)
        {
            ui->spectrum_plot_frame->show();
        }
        else
        {
            ui->spectrum_plot_frame->hide();
        }
    }
}

void Spectrum::FFT()
{

    double *t = new double[ui->windowSize_lineEdit->text().toInt()];

    for (int i=0; i<= ui->windowSize_lineEdit->text().toInt();i++)
    {
        t[i]=i*1/ui->lineEdit_fs->text().toInt();

        in[i] =0.7 *sin(2*M_PI*frq*t[i]);// generate sine waveform
        double multiplier = 0.5 * (1 - cos(2*M_PI*i/(windowSize-1)));//Hanning Window
        in[i] = multiplier * in[i];
    }

    for (int i=0; i<= ((windowSize/2)-1);i++)
    {
        ff[i]=Fs*i/windowSize;
    }
    plan_forward = fftw_plan_dft_r2c_1d ( windowSize, in, out, FFTW_ESTIMATE );

    fftw_execute ( plan_forward );

    double *v = new double[windowSize];

    for (int i = 0; i<= ((windowSize/2)-1); i++)
    {
        v[i]=(20*log(sqrt(out[i][0]*out[i][0]+ out[i][1]*out[i][1])))/N;  //Here   I  have calculated the y axis of the spectrum in dB
    }

    qDebug()<<"Step 4";
    QwtPlotCurve *curve = new QwtPlotCurve() ;
    curve->setSamples(ff,v,275);
    curve->setPen(Qt::red,1.5,Qt::DotLine);
    //graphMutex_Y.unlock();

    curve->attach(X_graphPlot);
    X_graphPlot->replot();
    X_graphPlot->show();

    fftw_destroy_plan ( plan_forward );
    fftw_free ( in );
    fftw_free ( out );

}

// void Spectrum::CreateSpectrumWidget()
// {
// #if 1
//     spectrogram = new Spectrogram(44100, 44100*100,512,1024);
//     spectrogramWidget = new QSpectrogram(spectrogram,NULL);
//     //ui->verticalLayout->addWidget(spectrogramWidget);
//     // spectrogramWidget->hide();
//     QObject::connect(this,SIGNAL(bufferFilled(float*,uint,double)),spectrogramWidget,SLOT(processData(float*,uint,double)));
//     resize(1024, 600);
// #else
//     m_PlotSpectrum = new QwtPlot();
//     m_PlotSpectrum->setTitle("SPectrum Plot");

//     m_SPectrumMatrix = new QwtMatrixRasterData();
//     spectrogram1 = new QwtPlotSpectrogram();
//     spectrogram1->attach(m_PlotSpectrum);
// #endif
// }

void Spectrum::on_pb_hide_show_menu_clicked()
{
    toggle_ctrl_btn = !toggle_ctrl_btn;
    if (toggle_ctrl_btn) {
        ui->frame_plot_control->show();
    } else {
        ui->frame_plot_control->hide();
    }
}


void Spectrum::on_pb_play_snap_shot_clicked()
{
    LOG_INFO("Spectrum::on_pb_play_snap_shot_clicked() <ENTER>");
    eth10G = EthernetSocket10G::getInstance();
    if (eth10G != nullptr)
    {
        unsigned int numByWrDone = 0;
        //memset(adcData_Point, '\0', 0x100000);

        if (ui->DDC_DataradioButton->isChecked())
        {
            unsigned int fifo_level = 0;

            if (ui->IOnly_radioButton->isChecked())
            {
                unsigned int byteToRd = 0;
                int dataWidth = ui->DataSize_comboBox->currentIndex();
                LOG_DEBUG("Data width index: %d", dataWidth);

                switch (dataWidth) {
                case 0: byteToRd = windowSize * 2; break;
                case 1: byteToRd = windowSize * 4; break;
                case 2: byteToRd = windowSize * 8; break;
                default: LOG_DEBUG("Unknown data width index: %d", dataWidth); break;
                }

                LOG_DEBUG("Bytes to read: %u", byteToRd);
                fifo_level = readRegisterValue(iface::eETH10G,0x2fc);
                fifo_level = BitUtils::extractBits15to0(fifo_level);
                LOG_DEBUG("Read Data Available: %ld", fifo_level);
                if (fifo_level >= byteToRd)
                {
                    QString filename = "StreamingData";
                    filename.append(QString::number(fifo_level));
                    filename.append(".bin");
                    FileReadWriteSetup(iface::eETH10G,byteToRd,"StreamingData.bin",eStream);
                    LOG_INFO("Sufficient FIFO level. Proceeding with MemoryReadFileInternal.");
                }
                else
                {
                    LOG_INFO("FIFO too low. Refresh rate may be too fast.");
                    ui->warning_label->setText("FIFO Empty Slow the Refresh Rate");
                }
            }
            if (ui->IQInterleved_radioButton->isChecked())
            {
                LOG_INFO("IQ Interleaved mode selected");
                unsigned int byteToRd = 0;
                int dataWidth = ui->DataSize_comboBox->currentIndex();
                LOG_INFO("Data width index: %d", dataWidth);

                switch (dataWidth) {
                case 0: byteToRd = windowSize * 4; break;
                case 1: byteToRd = windowSize * 8; break;
                case 2: byteToRd = windowSize * 16; break;
                default: LOG_INFO("Unknown data width index: %d", dataWidth); break;
                }

                LOG_DEBUG("Bytes to read: %u", byteToRd);
                fifo_level = readRegisterValue(iface::eETH10G,0x2fc);
                fifo_level = BitUtils::extractBits15to0(fifo_level);
                LOG_DEBUG("Read Data Available: %ld", fifo_level);
                if (fifo_level >= byteToRd)
                {
                    QString filename = "StreamingData";
                    filename.append(QString::number(fifo_level));
                    filename.append(".bin");
                    FileReadWriteSetup(iface::eETH10G,byteToRd,"StreamingData.bin",eStream);
                    LOG_INFO("Sufficient FIFO level. Proceeding with MemoryReadFileInternal.");
                }
                else
                {
                    LOG_INFO("FIFO too low. Refresh rate may be too fast.");
                    ui->warning_label->setText("FIFO Empty Slow the Refresh Rate");
                }
            }
        }
        ui->pb_plot->setEnabled(true);
        LOG_INFO("Plot button enabled");
    }
    else
    {
        LOG_INFO("No connection with FPGA 1G. Aborting snapshot.");
        QMessageBox::critical(this, "warning", "No Connection with FPGA 1G", QMessageBox::Ok);
        ui->pb_plot->setEnabled(false);
        plotTimer->stop();
    }

    LOG_INFO("Spectrum::on_pb_play_snap_shot_clicked() <EXIT>");
}

void Spectrum::FFT_Plot(int windowSize, fftw_complex* signal, fftw_complex* outBuffer)
{
    LOG_INFO("Spectrum::FFT_Plot(windowSize=%d) <ENTER>", windowSize);

    Fs = ui->lineEdit_fs->text().toInt();
    try {
        double* FF = new double[windowSize];
        for (int i = 0; i < windowSize; ++i)
            FF[i] = Fs * i / (windowSize - 1);

        //LOG_INFO("Frequency axis initialized with Fs = %.2f", Fs);

        plan_forward = fftw_plan_dft_1d(windowSize, signal, outBuffer, FFTW_FORWARD, FFTW_ESTIMATE);
        if (!plan_forward) {
            LOG_INFO("FFTW plan creation failed.");
            return;
        }
        fftw_execute(plan_forward);
        //LOG_INFO("FFTW execution completed.");

        double* temp = new double[windowSize];
        double* v = new double[windowSize];

        for (int i = 0; i < windowSize; ++i)
            temp[i] = std::sqrt(outBuffer[i][0] * outBuffer[i][0] + outBuffer[i][1] * outBuffer[i][1]);
        //LOG_INFO("Magnitude spectrum calculated.");

        if(ui->ChkBoxFFtShift->isChecked()){
            fftshift(temp,windowSize,sizeof(double));
        }

        if (maxHold) {
            LOG_INFO("MaxHold enabled. Shifting frequency axis.");
            int n2 = windowSize / 2;
            for (int i = 0; i < n2; ++i)
                std::swap(temp[i], temp[i + n2]);

            double xval = -Fs / 2;
            for (int i = 0; i < windowSize; ++i)
                FF[i] = xval, xval += Fs / windowSize;
            //LOG_INFO("Frequency axis shifted for MaxHold.");
        }

        double max = temp[0];
        double max_amplitude = 0;
        int maxIndex = 0;
        for (int i = 1; i < windowSize; ++i) {
            if (temp[i] > max) {
                max = temp[i];
                max_amplitude = max;
                maxIndex = i;
            }
        }
        //LOG_INFO("Peak magnitude at index %d with amplitude %.2f", maxIndex, max_amplitude);

        if (ui->enableWeight_checkBox->isChecked()) {
            max_amplitude = ui->enableWeight_lineEdit->text().toDouble();
            //LOG_INFO("User-defined weight override: %.2f", max_amplitude);
        }

        float* dataspectrum = new float[windowSize];
        for (int i = 0; i < windowSize; ++i) {
            double val = temp[i] / max_amplitude;
            v[i] = 20 * log10(val);
            dataspectrum[i] = val;
        }

        double max_db = v[maxIndex];

        if (ui->m_CBMaxHold->isChecked()) {
            //LOG_INFO("MaxHold plotting enabled.");
            if (!m_bMaxHoldEnable)
                m_vMaxHoldBuffer = new double[windowSize];

            for (int i = 0; i < windowSize; ++i) {
                double val = temp[i] / max_amplitude;
                v[i] = 20 * log10(val);
                if (!m_bMaxHoldEnable || m_vMaxHoldBuffer[i] < v[i])
                    m_vMaxHoldBuffer[i] = v[i];
            }

            m_bMaxHoldEnable = true;

            if (ui->IOnly_radioButton->isChecked()) {
                m_ObjMaxCurve->setSamples(FF, m_vMaxHoldBuffer, windowSize / 2);
                m_ObjMaxCurve->setPen(Qt::green, 1.5, Qt::SolidLine);
                LOG_INFO("MaxHold curve plotted (IOnly mode).");
            } else {
                m_ObjMaxCurve->setSamples(FF, m_vMaxHoldBuffer, windowSize);
                m_ObjMaxCurve->setPen(Qt::green, 1.5, Qt::SolidLine);
                LOG_INFO("MaxHold curve plotted (Full mode).");
            }

            m_ObjMaxCurve->attach(X_graphPlot);
        }

        if (ui->IOnly_radioButton->isChecked()) {
            curve_Y->setSamples(FF, v, windowSize / 2);
            curve_Y->setPen(Qt::yellow, 1.5, Qt::SolidLine);
            //LOG_INFO("Main curve plotted (IOnly mode).");
        } else {
            curve_Y->setSamples(FF, v, windowSize);
            curve_Y->setPen(Qt::yellow, 1.5, Qt::SolidLine);
            //LOG_INFO("Main curve plotted (Full mode).");
        }

        X_graphPlot->replot();
        //LOG_INFO("Graph replot triggered.");

        double freQuency = maxIndex * Fs / windowSize;
        ui->frequency_label->setText(QString::number(freQuency));
        ui->frequency_label_db->setText(QString::number(max_db));
        //LOG_INFO("Frequency label updated: %.2f Hz", freQuency);
        //LOG_INFO("dB label updated: %.2f dB", max_db);

        delete[] dataspectrum;
        delete[] FF;
        delete[] v;
        delete[] temp;
        LOG_INFO("Memory cleanup complete.");
    }
    catch (std::exception& e) {
        LOG_INFO("Exception occurred: %s", e.what());
    }

    LOG_INFO("FFT_Plot() <EXIT>");
}


void Spectrum::FFT_Plot(int windowSize, double* sample, fftw_complex* outBuffer)
{
    LOG_INFO("FFT_Plot() <ENTER>");
    LOG_INFO("Window size = %d", windowSize);
    Fs = ui->lineEdit_fs->text().toInt();

    for (int i = 0; i <= (N / 2 - 1); ++i) {
        ff[i] = Fs * i / N;
    }
    LOG_INFO("Frequency axis populated from Fs = %.2f", Fs);


    plan_forward = fftw_plan_dft_r2c_1d(windowSize, sample, outBuffer, FFTW_ESTIMATE);
    if (!plan_forward) {
        LOG_INFO("FFTW plan creation failed.");
        return;
    }
    LOG_INFO("FFTW plan created successfully.");


    fftw_execute(plan_forward);
    LOG_INFO("FFTW execution completed.");

    // Magnitude calculation
    double* v = new double[windowSize];
    for (int i = 0; i <= (windowSize / 2 - 1); ++i) {
        v[i] = sqrt(outBuffer[i][0] * outBuffer[i][0] + outBuffer[i][1] * outBuffer[i][1]);
    }
    LOG_INFO("Magnitude spectrum calculated.");

    // Plotting
    curve_Y->setSamples(ff, v, N / 2);
    curve_Y->setPen(Qt::yellow, 1.5, Qt::SolidLine);
    curve_Y->attach(X_graphPlot);
    X_graphPlot->replot();
    X_graphPlot->show();

    delete[] v;
    LOG_INFO("FFT_Plot() <EXIT>");
}

void Spectrum::on_pb_plot_with_file_clicked()
{
    if(ui->strmnStrt_radioButton->isChecked()==false)
    {
        QMessageBox::critical(this,"Not A Valid Setting","Please select Start Option",QMessageBox::Ok);
        return;
    }
    QString filter = "File Description (*.bin)";
    QString file_Name = QFileDialog::getOpenFileName(this, "Select a file...",QDir::currentPath(), filter);
    if(file_Name.isEmpty())
        return;
    FILE *fp ;
    if(!(fp= fopen(file_Name.toStdString().c_str(),"rb")))//StreamingData.bin","rb")))
    {
        return;
    }
    fseek(fp, 0, SEEK_END);
    dwFileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *read = new char[dwFileSize];
    fftw_complex *signal= new fftw_complex[windowSize];
    fftw_complex *outBuffer = new fftw_complex[windowSize];
    fread(read,1,dwFileSize,fp);
    fclose(fp);
    ofstream fc_debug_I;
    fc_debug_I.open("I_Data.txt",std::ios_base::out | std::ofstream::trunc);//("adc_data.txt", std::ios_base::out | std::ofstream::trunc);

    ofstream fc_debug_Q;
    fc_debug_Q.open("Q_Data.txt",std::ios_base::out | std::ofstream::trunc);//("adc_data.txt", std::ios_base::out | std::ofstream::trunc);

    if(ui->strmnStrt_radioButton->isChecked())
    {
        if(ui->DDC_DataradioButton->isChecked())
        {
            qDebug()<<"DDC select";
            qDebug()<<"Window Size"<<windowSize;
            qDebug()<<"Fs "<<Fs;
            if(ui->IQInterleved_radioButton->isChecked())
            {
                /// IQ 16
                if(ui->DataSize_comboBox->currentIndex()==0)
                {
                    for(int i=0;i<windowSize;i++)
                    {
                        short temp;
                        short temp2;
                        double multiplier = 0.5 * (1 - cos(2*M_PI*i/(windowSize-1)));//Hanning Window
                        temp=((unsigned char)read[4*i+0]<<8)&0xFF00;
                        temp |= (unsigned char)read[4*i+1];
                        temp2=((unsigned char)read[4*i+2]<<8)&0xFF00;
                        temp2 |= (unsigned char)read[4*i+3];
                        // signal[i][0] = (double)temp2*multiplier;
                        // signal[i][1] = (double)temp*multiplier;

                        signal[i][1] = (double)temp2*multiplier;
                        signal[i][0] = (double)temp*multiplier;

                        fc_debug_I<<std::hex << temp2<<endl;
                        fc_debug_Q<<std::hex << temp <<endl;

                    }
                }
                /// IQ 32
                if(ui->DataSize_comboBox->currentIndex()==1)
                {
                    for(int i=0;i<windowSize;i++)
                    {
                        long int temp;
                        long int temp2;
                        double multiplier = 0.5 * (1 - cos(2*M_PI*i/(windowSize-1)));//Hanning Window
                        temp  =((unsigned char)read[8*i+0]<<24)&0xFF000000;
                        temp |=((unsigned char)read[8*i+1]<<16)&0xFF0000;
                        temp |=((unsigned char)read[8*i+2]<<8)&0xFF00;
                        temp |=((unsigned char)read[8*i+3])&0xFF;

                        temp2  =((unsigned char)read[8*i+4]<<24)&0xFF000000;
                        temp2 |=((unsigned char)read[8*i+5]<<16)&0xFF0000;
                        temp2 |=((unsigned char)read[8*i+6]<<8)&0xFF00;
                        temp2 |=((unsigned char)read[8*i+7])&0xFF;


                        signal[i][0] = (double)temp2*multiplier;
                        signal[i][1] = (double)temp*multiplier;
                        qDebug()<<"Temp ="<<temp;
                        qDebug()<<"Temp ="<<temp2;
                        fc_debug_Q<<temp<<endl;
                        fc_debug_I<<temp2<<endl;

                    }
                }
                /// IQ 64
                if(ui->DataSize_comboBox->currentIndex()==2)
                {
                    for(int i=0;i<windowSize;i++)
                    {
                        long long temp;
                        long long temp2;
                        double multiplier = 0.5 * (1 - cos(2*M_PI*i/(windowSize-1)));//Hanning Window
                        temp  =((unsigned char)read[16*i]<<56)&0xFF00000000000000;
                        temp |=((unsigned char)read[16*i+1]<<48)&0xFF000000000000;
                        temp |=((unsigned char)read[16*i+2]<<40)&0xFF0000000000;
                        temp |=((unsigned char)read[16*i+3]<<32)&0xFF00000000;
                        temp |=((unsigned char)read[16*i+4]<<24)&0xFF000000;
                        temp |=((unsigned char)read[16*i+5]<<16)&0xFF0000;
                        temp |=((unsigned char)read[16*i+6]<<8)&0xFF00;
                        temp |=((unsigned char)read[16*i+7]);

                        temp2  =((unsigned char)read[16*i+8]<<56)&0xFF00000000000000;
                        temp2 |=((unsigned char)read[16*i+9]<<48)&0xFF000000000000;
                        temp2 |=((unsigned char)read[16*i+10]<<40)&0xFF0000000000;
                        temp2 |=((unsigned char)read[16*i+11]<<32)&0xFF00000000;
                        temp2 |=((unsigned char)read[16*i+12]<<24)&0xFF000000;
                        temp2 |=((unsigned char)read[16*i+13]<<16)&0xFF0000;
                        temp2 |=((unsigned char)read[16*i+14]<<8)&0xFF00;
                        temp2 |=((unsigned char)read[16*i+15]);


                        signal[i][0] = (double)temp2*multiplier;
                        signal[i][1] = (double)temp*multiplier;

                        fc_debug_I<<temp<<endl;
                        fc_debug_Q<<temp2<<endl;

                    }
                }
            }

            if(ui->IOnly_radioButton->isChecked())
            {
                qDebug()<<"I Only ";
                /// IQ 16
                if(ui->DataSize_comboBox->currentIndex()==0)
                {
                    qDebug()<<"I Only data size 16";
                    for(int i=0;i<windowSize;i++)
                    {
                        short temp;
                        short temp2=0;
                        double multiplier = 0.5 * (1 - cos(2*M_PI*i/(windowSize-1)));//Hanning Window
                        temp=((unsigned char)read[2*i+1]<<8)&0xFF00;
                        temp |= (unsigned char)read[2*i];
                        //                        temp2=((unsigned char)read[2*i+2]<<8)&0xFF00;
                        //                        temp2 |= (unsigned char)read[2*i+3];
                        signal[i][0] = (double)temp2*multiplier;
                        signal[i][1] = (double)temp*multiplier;

                        fc_debug_I<<temp<<endl;
                        fc_debug_Q<<temp2<<endl;

                    }
                }
                /// IQ 32
                if(ui->DataSize_comboBox->currentIndex()==1)
                {
                    for(int i=0;i<windowSize;i++)
                    {
                        qint32 temp;
                        qint32 temp2=0;
                        double multiplier = 0.5 * (1 - cos(2*M_PI*i/(windowSize-1)));//Hanning Window
                        temp  =((unsigned char)read[4*i+0]<<24)&0xFF000000;
                        temp |=((unsigned char)read[4*i+1]<<16)&0xFF0000;
                        temp |=((unsigned char)read[4*i+2]<<8)&0xFF00;
                        temp |=((unsigned char)read[4*i+3]);

                        //                        temp2  =((unsigned char)read[8*i+4]<<24)&0xFF000000;
                        //                        temp2 |=((unsigned char)read[8*i+5]<<16)&0xFF0000;
                        //                        temp2 |=((unsigned char)read[8*i+6]<<8)&0xFF00;
                        //                        temp2 |=((unsigned char)read[8*i+7]);

                        signal[i][0] = (double)temp2*multiplier;
                        signal[i][1] = (double)temp*multiplier;
                        fc_debug_I<<temp<<endl;
                        fc_debug_Q<<temp2<<endl;

                    }
                }
                /// IQ 64
                if(ui->DataSize_comboBox->currentIndex()==2)
                {
                    for(int i=0;i<windowSize;i++)
                    {
                        long long temp;
                        long long temp2=0;
                        double multiplier = 0.5 * (1 - cos(2*M_PI*i/(windowSize-1)));//Hanning Window
                        temp  =((unsigned char)read[8*i]<<56)&0xFF00000000000000;
                        temp |=((unsigned char)read[8*i+1]<<48)&0xFF000000000000;
                        temp |=((unsigned char)read[8*i+2]<<40)&0xFF0000000000;
                        temp |=((unsigned char)read[8*i+3]<<32)&0xFF00000000;
                        temp |=((unsigned char)read[8*i+4]<<24)&0xFF000000;
                        temp |=((unsigned char)read[8*i+5]<<16)&0xFF0000;
                        temp |=((unsigned char)read[8*i+6]<<8)&0xFF00;
                        temp |=((unsigned char)read[8*i+7]);

                        //                        temp2  =((unsigned char)read[16*i+8]<<56)&0xFF00000000000000;
                        //                        temp2 |=((unsigned char)read[16*i+9]<<48)&0xFF000000000000;
                        //                        temp2 |=((unsigned char)read[16*i+10]<<40)&0xFF0000000000;
                        //                        temp2 |=((unsigned char)read[16*i+11]<<32)&0xFF00000000;
                        //                        temp2 |=((unsigned char)read[16*i+12]<<24)&0xFF000000;
                        //                        temp2 |=((unsigned char)read[16*i+13]<<16)&0xFF0000;
                        //                        temp2 |=((unsigned char)read[16*i+14]<<8)&0xFF00;
                        //                        temp2 |=((unsigned char)read[16*i+15]);


                        signal[i][0] = (double)temp2*multiplier;
                        signal[i][1] = (double)temp*multiplier;
                        fc_debug_I<<temp<<endl;
                        fc_debug_Q<<temp2<<endl;

                    }
                }
            }
        }
    }
    FFT_Plot(windowSize,signal,outBuffer);
    delete read;
    delete [] signal;
    delete [] outBuffer;
    fc_debug_I.close();
    fc_debug_Q.close();
}

void Spectrum::playFile()
{
    LOG_INFO("Spectrum::playFile() <ENTER> ");
    LOG_INFO("dwFileSize = %llu, playBack_Counter = %llu", dwFileSize, playBack_Counter);

    if (dwFileSize <= playBack_Counter)
    {
        LOG_INFO("End of file reached. Resetting playback.");

        playBack_Counter = 0;
        fseek(filePlayBack_fp, 0, SEEK_SET);
        playBack_Timer->stop();
        fclose(filePlayBack_fp);
        playBack_Counter = 0;

        ui->m_PBPlayRecordData->setText("Start Playing  Recorded Data");
        FilePlay = false;
        ui->strmnStrt_radioButton->setDisabled(false);
        ui->strmnStop_radioButton->setDisabled(false);

        LOG_INFO("Playback stopped. UI reset. File closed.");
        return;
    }
    LOG_INFO("Seeking to playBack_Counter = %llu", playBack_Counter);
    fseek(filePlayBack_fp, playBack_Counter, SEEK_SET);

    LOG_INFO("Allocating buffers: read[0x400000], signal[%d], outBuffer[%d]", windowSize, windowSize);
    char *read = new char[0x400000];
    fftw_complex *signal = new fftw_complex[windowSize];
    fftw_complex *outBuffer = new fftw_complex[windowSize];

    size_t bytesRead = fread(read, 1, 8 * windowSize, filePlayBack_fp);
    LOG_INFO("Read %zu bytes from file", bytesRead);

    if (ui->strmnStrt_radioButton->isChecked())
    {
        LOG_INFO("Stream Start mode selected");

        if (ui->DDC_DataradioButton->isChecked())
        {
            LOG_INFO("DDC Data mode selected");
            LOG_INFO("Window Size = %d", windowSize);
            LOG_INFO("Sampling Frequency Fs = %.2f", Fs);

            if (ui->IQInterleved_radioButton->isChecked())
            {
                LOG_INFO("Mode: IQ Interleaved selected");

                int dataSizeIndex = ui->DataSize_comboBox->currentIndex();
                bool byteReversed = ui->byteReverse_checkBox->isChecked();
                LOG_INFO("Data Size Index: %d, Byte Reversed: %s", dataSizeIndex, byteReversed ? "true" : "false");

                for (int i = 0; i < windowSize; ++i) {
                    double multiplier = 0.5 * (1 - cos(2 * M_PI * i / (windowSize - 1)));
                    const unsigned char* base = reinterpret_cast<const unsigned char*>(read);

                    if (dataSizeIndex == 0) { // IQ 16
                        LOG_INFO("Processing IQ Interleaved - 16-bit samples");
                        short I = parse16(base + 4 * i, byteReversed);
                        short Q = parse16(base + 4 * i + 2, byteReversed);

                        signal[i][0] = Q * multiplier;
                        signal[i][1] = I * multiplier;

                        LOG_INFO("Index %d: I=%d, Q=%d, Multiplier=%.6f, Signal=[%.3f, %.3f]",
                                 i, I, Q, multiplier, signal[i][0], signal[i][1]);
                    }
                    else if (dataSizeIndex == 1) { // IQ 32
                        LOG_INFO("Processing IQ Interleaved - 32-bit samples");
                        qint32 I = parse32(base + 8 * i, byteReversed);
                        qint32 Q = parse32(base + 8 * i + 4, byteReversed);

                        signal[i][0] = Q * multiplier;
                        signal[i][1] = I * multiplier;

                        LOG_DEBUG("Index %d: I=%d, Q=%d, Multiplier=%.6f, Signal=[%.3f, %.3f]",
                                  i, I, Q, multiplier, signal[i][0], signal[i][1]);
                    }
                    else if (dataSizeIndex == 2) { // IQ 64
                        LOG_INFO("Processing IQ Interleaved - 64-bit samples");
                        long long I = parse64(base + 16 * i, byteReversed);
                        long long Q = parse64(base + 16 * i + 8, byteReversed);

                        signal[i][0] = Q * multiplier;
                        signal[i][1] = I * multiplier;

                        LOG_DEBUG("Index %d: I=%lld, Q=%lld, Multiplier=%.6f, Signal=[%.3f, %.3f]",
                                  i, I, Q, multiplier, signal[i][0], signal[i][1]);
                    }
                    else {
                        LOG_ERROR("Unknown data size index: %d", dataSizeIndex);
                    }
                }
            }
            if (ui->IOnly_radioButton->isChecked())
            {
                LOG_INFO("Mode: I Only selected");
                int dataSizeIndex = ui->DataSize_comboBox->currentIndex();
                bool byteReversed = ui->byteReverse_checkBox->isChecked();
                LOG_INFO("Data Size Index: %d, Byte Reversed: %s", dataSizeIndex, byteReversed ? "true" : "false");

                for (int i = 0; i < windowSize; ++i)
                {
                    double multiplier = 0.5 * (1 - cos(2 * M_PI * i / (windowSize - 1)));
                    const unsigned char* base = reinterpret_cast<const unsigned char*>(read);

                    if (dataSizeIndex == 0) { // I 16
                        LOG_INFO("Processing I Only - 16-bit samples");
                        short I = parse16(base + 2 * i, byteReversed);

                        signal[i][0] = 0;
                        signal[i][1] = I * multiplier;

                        LOG_INFO("Index %d: I=%d, Multiplier=%.6f, Signal=[%.3f, %.3f]",
                                 i, I, multiplier, signal[i][0], signal[i][1]);
                    }
                    else if (dataSizeIndex == 1) { // I 32
                        LOG_INFO("Processing I Only - 32-bit samples");
                        qint32 I = parse32(base + 4 * i, byteReversed);

                        signal[i][0] = 0;
                        signal[i][1] = I * multiplier;

                        LOG_INFO("Index %d: I=%d, Multiplier=%.6f, Signal=[%.3f, %.3f]",
                                 i, I, multiplier, signal[i][0], signal[i][1]);
                    }
                    else if (dataSizeIndex == 2) { // I 64
                        LOG_INFO("Processing I Only - 64-bit samples");
                        long long I = parse64(base + 8 * i, byteReversed);

                        signal[i][0] = 0;
                        signal[i][1] = I * multiplier;

                        LOG_INFO("Index %d: I=%lld, Multiplier=%.6f, Signal=[%.3f, %.3f]",
                                 i, I, multiplier, signal[i][0], signal[i][1]);
                    }
                    else {
                        LOG_ERROR("Unknown data size index: %d", dataSizeIndex);
                    }
                }
            }
        }
        FFT_Plot(windowSize,signal,outBuffer);
    }

    delete[] read;
    delete[] signal;
    delete[] outBuffer;

    playBack_Counter += 8 * windowSize;
    LOG_INFO("Updated playBack_Counter = %llu", playBack_Counter);

    int progressPercent = (playBack_Counter * 100) / dwFileSize;
    ui->progressBar->setValue(progressPercent);
    LOG_INFO("Progress bar updated: %d", progressPercent);

    if (progressPercent >= 100)
    {
        LOG_INFO("Playback complete. Resetting UI state.");
        ui->m_PBPlayRecordData->setText("Start Playing  Recorded Data");
        FilePlay = false;
        ui->strmnStrt_radioButton->setDisabled(false);
        ui->strmnStop_radioButton->setDisabled(false);
    }
}

void Spectrum::on_m_PBPlayRecordData_clicked()
{
    LOG_INFO("Spectrum::on_m_PBPlayRecordData_clicked() <ENTER>");

    if (!ui->strmnStrt_radioButton->isChecked()) {
        QMessageBox::critical(this, "Warning", "Please select Start option before playing recorded data from file", QMessageBox::Ok);
        LOG_ERROR("Start option not selected. Playback aborted.");
        return;
    }

    if (!FilePlay) {
        LOG_TO_FILE("Playback not active. Attempting to start playback.");

        QString filter = "File Description (*.bin;*.hex;)";
        QString fileName = QFileDialog::getOpenFileName(this, "Select a file...", QDir::currentPath(), filter);
        if (fileName.isEmpty()) {
            QMessageBox::information(this, "Information", "Please select a file to play recorded data", QMessageBox::Ok);
            LOG_ERROR("No file selected. Playback aborted.");
            return;
        }

        LOG_TO_FILE("Selected file: %s", fileName.toStdString().c_str());

        playBack_Counter = 0;
        filePlayBack_fp = fopen(fileName.toStdString().c_str(), "rb");
        if (!filePlayBack_fp) {
            LOG_TO_FILE("Failed to open file: %s", fileName.toStdString().c_str());
            return;
        }

        fseek(filePlayBack_fp, 0, SEEK_END);
        dwFileSize = ftell(filePlayBack_fp);
        fseek(filePlayBack_fp, 0, SEEK_SET);
        LOG_TO_FILE("File opened. Size = %u bytes", dwFileSize);

        int refreshRate = ui->refrestRate_lineEdit->text().toInt();
        LOG_TO_FILE("Refresh rate set to %d ms", refreshRate);

        ui->progressBar->setMinimum(0);
        ui->progressBar->setMaximum(100);
        ui->progressBar->setTextVisible(true);
        ui->progressBar->setFormat("%p%");
        playBack_Timer = new QTimer(this);
        playBack_Timer->setInterval(refreshRate);
        connect(playBack_Timer, SIGNAL(timeout()), this, SLOT(playFile()));
        playBack_Timer->start();
        LOG_TO_FILE("Playback timer started");

        ui->m_PBPlayRecordData->setText("Stop Playing Recorded Data");
        FilePlay = true;

        ui->strmnStrt_radioButton->setDisabled(true);
        ui->strmnStop_radioButton->setDisabled(true);
        LOG_TO_FILE("Start/Stop radio buttons disabled during playback");
    }
    else {
        LOG_TO_FILE("Playback active. Stopping playback.");

        playBack_Timer->stop();
        fclose(filePlayBack_fp);
        playBack_Counter = 0;

        ui->progressBar->setValue(0);
        ui->m_PBPlayRecordData->setText("Start Playing Recorded Data");

        ui->strmnStrt_radioButton->setDisabled(false);
        ui->strmnStop_radioButton->setDisabled(false);
        ui->IOnly_radioButton->setDisabled(false);
        ui->IQInterleved_radioButton->setDisabled(false);
        //ui->raw_radioButton->setDisabled(false);
        ui->DDC_DataradioButton->setDisabled(false);

        FilePlay = false;
        LOG_TO_FILE("Playback stopped and UI controls re-enabled");
    }
    LOG_TO_FILE("Spectrum::on_m_PBPlayRecordData_clicked() <EXIT>");
}

void Spectrum::on_pb_plot_clicked()
{

    if(windowSize<256 && windowSize>8192)
    {
        QMessageBox::critical(this,"warning","invalid Window SIze \n Out Of Range of 256 to 8192",QMessageBox::Ok);
        return;
    }
    qDebug()<<"plot enter"<<windowSize;

    FILE *fp =fopen("StreamingData.bin","rb");
    if(fp==NULL )
    {
        return;
    }
    fseek(fp, 0, SEEK_END);
    dwFileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *read = new char[dwFileSize];
    fftw_complex *signal= new fftw_complex[windowSize];
    fftw_complex *outBuffer = new fftw_complex[windowSize];
    fread(read,1,dwFileSize,fp);
    fclose(fp);
    ofstream fc_debug_I;
    fc_debug_I.open("I_Data.txt",std::ios_base::out | std::ofstream::trunc);//("adc_data.txt", std::ios_base::out | std::ofstream::trunc);

    ofstream fc_debug_Q;
    fc_debug_Q.open("Q_Data.txt",std::ios_base::out | std::ofstream::trunc);//("adc_data.txt", std::ios_base::out | std::ofstream::trunc);

    if(ui->strmnStrt_radioButton->isChecked())
    {
        if(ui->DDC_DataradioButton->isChecked())
        {
            qDebug()<<"DDC select";
            qDebug()<<"Window Size"<<windowSize;
            qDebug()<<"Fs "<<Fs;


            if(ui->IQInterleved_radioButton->isChecked())
            {
                /// IQ 16
                if(ui->DataSize_comboBox->currentIndex()==0)
                {
                    for(int i=0;i<windowSize;i++)
                    {
                        short temp;
                        short temp2;
                        double multiplier = 0.5 * (1 - cos(2*M_PI*i/(windowSize-1)));//Hanning Window
                        {
                            temp=((unsigned char)read[4*i]<<8)&0xFF00;
                            temp |= (unsigned char)read[4*i+1];
                            temp2=((unsigned char)read[4*i+2]<<8)&0xFF00;
                            temp2 |= (unsigned char)read[4*i+3];
                            signal[i][0] = (double)temp*multiplier;
                            signal[i][1] = (double)temp2*multiplier;
                        }
                        //fc_debug_I << std::hex << temp2 <<endl;
                        //fc_debug_Q << std::hex << temp << endl;
                    }

                }
                /// IQ 32
                if(ui->DataSize_comboBox->currentIndex()==1)
                {
                    qDebug()<<"IQ interleaved 32";
                    for(int i=0;i<windowSize;i++)
                    {

                        int temp;
                        int temp2;
                        double multiplier = 0.5 * (1 - cos(2*M_PI*i/(windowSize-1)));//Hanning Window
                        {
                            temp  =((unsigned char)read[8*i+0]<<24)&0xFF000000;
                            temp |=((unsigned char)read[8*i+1]<<16)&0xFF0000;
                            temp |=((unsigned char)read[8*i+2]<<8)&0xFF00;
                            temp |=((unsigned char)read[8*i+3]);

                            temp2  =((unsigned char)read[8*i+4]<<24)&0xFF000000;
                            temp2 |=((unsigned char)read[8*i+5]<<16)&0xFF0000;
                            temp2 |=((unsigned char)read[8*i+6]<<8)&0xFF00;
                            temp2 |=((unsigned char)read[8*i+7]);


                            signal[i][0] = (double)temp*multiplier;
                            signal[i][1] = (double)temp2*multiplier;
                        }
                        fc_debug_I << std::hex << temp2 <<endl;
                        fc_debug_Q << std::hex << temp << endl;
                    }

                }
                /// IQ 64
                if(ui->DataSize_comboBox->currentIndex()==2)
                {
                    for(int i=0;i<windowSize;i++)
                    {
                        // const int nConstShift = 56;
                        long long temp;
                        long long temp2;
                        double multiplier = 0.5 * (1 - cos(2*M_PI*i/(windowSize-1)));//Hanning Window

                        //temp  =((unsigned long long )read[16*i] << 56)&0xFF00000000000000;


                        temp  =((unsigned char)read[16*i] << 56)&0xFF00000000000000;
                        temp |=((unsigned char)read[16*i+1]<<48)&0xFF000000000000;
                        temp |=((unsigned char)read[16*i+2]<<40)&0xFF0000000000;
                        temp |=((unsigned char)read[16*i+3]<<32)&0xFF00000000;
                        temp |=((unsigned char)read[16*i+4]<<24)&0xFF000000;
                        temp |=((unsigned char)read[16*i+5]<<16)&0xFF0000;
                        temp |=((unsigned char)read[16*i+6]<<8)&0xFF00;
                        temp |=((unsigned char)read[16*i+7]);

                        temp2  =((unsigned char)read[16*i+8]<<56)&0xFF00000000000000;
                        temp2 |=((unsigned char)read[16*i+9]<<48)&0xFF000000000000;
                        temp2 |=((unsigned char)read[16*i+10]<<40)&0xFF0000000000;
                        temp2 |=((unsigned char)read[16*i+11]<<32)&0xFF00000000;
                        temp2 |=((unsigned char)read[16*i+12]<<24)&0xFF000000;
                        temp2 |=((unsigned char)read[16*i+13]<<16)&0xFF0000;
                        temp2 |=((unsigned char)read[16*i+14]<<8)&0xFF00;
                        temp2 |=((unsigned char)read[16*i+15]);


                        signal[i][0] = (double)temp2*multiplier;
                        signal[i][1] = (double)temp*multiplier;

                    }
                }
            }
            if(ui->IOnly_radioButton->isChecked())
            {
                qDebug()<<"I Only ";
                /// I 16
                if(ui->DataSize_comboBox->currentIndex()==0)
                {
                    qDebug()<<"I Only data size 16";
                    for(int i=0;i<windowSize;i++)
                    {
                        short temp;
                        short temp2=0;
                        double multiplier = 0.5 * (1 - cos(2*M_PI*i/(windowSize-1)));//Hanning Window
                        if(ui->byteReverse_checkBox->isChecked())
                        {
                            temp=((unsigned char)read[2*i+1]<<8)&0xFF00;
                            temp |= (unsigned char)read[2*i];
                            signal[i][0] = (double)temp2*multiplier;
                            signal[i][1] = (double)temp*multiplier;
                        }
                        else
                        {
                            temp=((unsigned char)read[2*i]<<8)&0xFF00;
                            temp |= (unsigned char)read[2*i+1];
                            //                        temp2=((unsigned char)read[2*i+2]<<8)&0xFF00;
                            //                        temp2 |= (unsigned char)read[2*i+3];
                            signal[i][0] = (double)temp2*multiplier;
                            signal[i][1] = (double)temp*multiplier;
                        }

                    }
                }
                /// I 32
                if(ui->DataSize_comboBox->currentIndex()==1)
                {
                    qDebug()<<"I Only 32"<<windowSize;
                    for(int i=0;i<windowSize;i++)
                    {
                        int temp;
                        int temp2=0;
                        double multiplier = 0.5 * (1 - cos(2*M_PI*i/(windowSize-1)));//Hanning Window
                        if(ui->byteReverse_checkBox->isChecked())
                        {
                            temp  =((unsigned char)read[4*i+3]<<24)&0xFF000000;
                            temp |=((unsigned char)read[4*i+2]<<16)&0xFF0000;
                            temp |=((unsigned char)read[4*i+1]<<8)&0xFF00;
                            temp |=((unsigned char)read[4*i]);
                            signal[i][0] = (double)temp2*multiplier;
                            signal[i][1] = (double)temp*multiplier;
                        }
                        else
                        {
                            temp  =((unsigned char)read[4*i]<<24)&0xFF000000;
                            temp |=((unsigned char)read[4*i+1]<<16)&0xFF0000;
                            temp |=((unsigned char)read[4*i+2]<<8)&0xFF00;
                            temp |=((unsigned char)read[4*i+3]);
                            signal[i][0] = (double)temp2*multiplier;
                            signal[i][1] = (double)temp*multiplier;
                        }

                    }
                }
                /// I 64
                if(ui->DataSize_comboBox->currentIndex()==2)
                {
                    for(int i=0;i<windowSize;i++)
                    {
                        long long temp;
                        long long temp2=0;
                        double multiplier = 0.5 * (1 - cos(2*M_PI*i/(windowSize-1)));//Hanning Window
                        temp  =((unsigned char)read[8*i]<<56)&0xFF00000000000000;
                        temp |=((unsigned char)read[8*i+1]<<48)&0xFF000000000000;
                        temp |=((unsigned char)read[8*i+2]<<40)&0xFF0000000000;
                        temp |=((unsigned char)read[8*i+3]<<32)&0xFF00000000;
                        temp |=((unsigned char)read[8*i+4]<<24)&0xFF000000;
                        temp |=((unsigned char)read[8*i+5]<<16)&0xFF0000;
                        temp |=((unsigned char)read[8*i+6]<<8)&0xFF00;
                        temp |=((unsigned char)read[8*i+7]);
                        signal[i][0] = (double)temp2*multiplier;
                        signal[i][1] = (double)temp*multiplier;

                    }
                }
            }
        }
        FFT_Plot(windowSize,signal,outBuffer);
        fc_debug_I.close();
        fc_debug_Q.close();
        delete [] read;
        delete [] signal;
        delete [] outBuffer;
    }
   // exit(0);
}

void Spectrum::on_m_PBGetADCData_clicked()
{
    LOG_INFO("Spectrum::on_m_PBGetADCData_clicked() <ENTER>");

    // Extract window size from UI
    int windowSize = ui->windowSize_lineEdit->text().toInt();
    LOG_INFO("Parsed window size from UI: %d", windowSize);

    // Configure thread parameters
    //obj_thread->functionType = 0;
   // obj_thread->windowSize = windowSize;
    LOG_DEBUG("Thread configured: functionType=0, windowSize=%d", windowSize);

    // Start acquisition thread
    //obj_thread->eInterFace = eETH10G;
    //obj_thread->start();
    LOG_DEBUG("Acquisition thread started");

    // Show progress dialog
    stat_Message = new QMessageBox(this);
    stat_Message->setIcon(QMessageBox::Warning);
    stat_Message->setStandardButtons(QMessageBox::Cancel);
    stat_Message->setText("Write In Progress ...");
    stat_Message->show();
    LOG_TO_FILE("Progress dialog shown: 'Write In Progress ...'");

    // Connect cancel button to thread cancellation
    connect(stat_Message->button(QMessageBox::Cancel), SIGNAL(clicked()), this, SLOT(cancelThread()));
    LOG_DEBUG("Cancel button connected to cancelThread()");
    LOG_INFO("Spectrum::on_m_PBGetADCData_clicked() <EXIT>");
}
void Spectrum::cancelThread()
{
   // obj_thread->StopThread = true;
}
void Spectrum::UpdateProgressBAR(qint64 RecvDataDone)
{
    ui->progressBar->setValue(RecvDataDone);
    LOG_INFO("Updating progress bar");
}

void Spectrum::on_chnl_comboBox_currentIndexChanged(int index)
{

}
void Spectrum::FileReadWriteSetup(iface deviceType, uint iFileSize, QString sFilePath, eXferDir dir)
{

    LOG_INFO("Spectrum::FileReadWriteSetup()<ENTER>");
    char* byArrPkt = nullptr;
    Proto protocolobj;
    switch (deviceType)
    {
    case iface::eSERIAL:
    {
        serial = UartSerial::getInstance();
        if (!serial)
        {
            LOG_TO_FILE("ERROR: Serial pointer is null.");
            return;
        }
        break;
    }
    case iface::eETH1G:
    {
        stFileReadWriteConf Cnf;
        Cnf.iFileSize = iFileSize;
        Cnf.sFilePath = sFilePath;
        Cnf.eInterface = iface::eETH10G;
        Cnf._Dir = dir;
        setupTransferAgent->configure(Cnf);

    }
    break;
    case iface::ePCIe:
        break;
    case iface::eETH10G:
    {
        stFileReadWriteConf Cnf;
        Cnf.iFileSize = iFileSize;
        Cnf.sFilePath = sFilePath;
        Cnf.eInterface = iface::eETH10G;
        Cnf._Dir = dir;
        LOG_INFO("eETH10G: iFileSize:%d,sFilePath:%s",iFileSize,sFilePath.toStdString().c_str());
        setupTransferAgent->configure(Cnf);
        setupTransferAgent->start();
    }
    break;
    default:
        LOG_INFO("No valid interface selection");
    }
    delete byArrPkt;
    LOG_INFO("DeviceSetup::FileReadWriteSetup()<EXIT>");
}

void Spectrum::handleRegisterWrite(iface deviceType, uint iaddr, uint ival)
{
    LOG_INFO("Spectrum::handleRegisterWrite() <ENTER>");
    LOG_INFO("Addr:0x%08X, value:0x%08X",iaddr,ival);
    char* byArrPkt = nullptr;

    Proto protocolobj;
    int pktLen = protocolobj.mPktRegWrite(iaddr, ival, &byArrPkt);
    switch (deviceType)
    {
    case iface::eSERIAL:
    {
        serial = UartSerial::getInstance();
        if (!serial) {
            LOG_TO_FILE("ERROR: Serial pointer is null.");
            return;
        }
        if(!serial->sendData(byArrPkt, pktLen)){
            LOG_TO_FILE("Sent filed!!!<Serial>");
        }
        break;
    }

    case iface::eETH1G:
    {
        eth1G = EthernetSocket::getInstance();
        if (!eth1G) {
            LOG_TO_FILE("ERROR: Ethernet pointer is null.");
            return;
        }
        if(!eth1G->sendData(byArrPkt,pktLen,eth1G->RemoteIP.toStdString(),eth1G->Port)){
            LOG_TO_FILE("Sent filed!!!<eth1G>");
        }
        {
            char ByteArr64BitPakt[64]={0};
            std::string senderIp;
            uint16_t senderport;
            //Read and discard the packet
            if(eth10G->receiveData(ByteArr64BitPakt,pktLen,senderIp,senderport))
            {
                int reg_val = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
                LOG_TO_FILE("RegVal:0x%08X",reg_val);

            }else{
                LOG_TO_FILE("Receive filed!!!<eth10G>");
            }
        }
        break;
    }
    case eETH10G:
        eth10G = EthernetSocket10G::getInstance();
        if (!eth10G) {
            LOG_TO_FILE("ERROR: Ethernet pointer is null.");
            return;
        }
        if(!eth10G->sendData(byArrPkt,pktLen,eth10G->RemoteIP.toStdString(),eth10G->Port)){
            LOG_TO_FILE("Sent filed!!!<eth1G>");
        }
        {
            char ByteArr64BitPakt[64]={0};
            std::string senderIp;
            uint16_t senderport;
            //Read and discard the packet
            if(eth10G->receiveData(ByteArr64BitPakt,pktLen,senderIp,senderport))
            {
                int reg_val = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
                LOG_TO_FILE("RegVal:0x%08X",reg_val);

            }else{
                LOG_TO_FILE("Receive filed!!!<eth10G>");
            }
        }
        break;
    case ePCIe:
        break;
    case eNONE:
        break;
    }
    LOG_INFO("MainWindow::handleRegisterWrite() <EXIT>");
}

uint Spectrum::readRegisterValue(iface deviceType,uint addr)
{
    LOG_TO_FILE(" DeviceSetup::readRegisterValue() <ENTER>");
    char* byArrPkt = nullptr;
    uint reg_val = -20;
    char ByteArr64BitPakt[64];
    Proto protocolobj;
    int pktLen = protocolobj.mPktRegRead(addr, &byArrPkt);
    switch (deviceType)
    {
    case iface::eSERIAL:
    {
        serial = UartSerial::getInstance();
        if (!serial)
        {
            LOG_TO_FILE("ERROR: Serial pointer is null.");
            return -1;
        }
        if(!serial->sendData(byArrPkt, pktLen))
        {
            LOG_TO_FILE("Sent filed!!!<Serial>");
        }
        else
        {
            serial->sendData(byArrPkt, pktLen);
            if(serial->receiveData(ByteArr64BitPakt, pktLen)){
                int reg_val = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
                LOG_TO_FILE("REG_VAL:0x%08X",reg_val);
            }
        }
        break;
    }
    case iface::eETH1G:
    {
        eth1G = EthernetSocket::getInstance();
        if (!eth1G) {
            LOG_TO_FILE("ERROR: Ethernet pointer is null.");
            return -1;
        }
        if(!eth1G->sendData(byArrPkt,pktLen,eth1G->RemoteIP.toStdString(),eth1G->Port)){
            LOG_TO_FILE("Sent filed!!!<eth1G>");
        }
        else
        {
            std::string senderIp;
            uint16_t senderport;
            if(eth1G->receiveData(ByteArr64BitPakt,pktLen,senderIp,senderport))
            {
                reg_val = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
                LOG_TO_FILE("REG_VAL:0x%08X",reg_val);
            }else{
                LOG_TO_FILE("Receive filed!!!<eth1G>");
            }
        }
    }
    break;
    case iface::ePCIe:
        break;
    case iface::eETH10G:
    {
        eth10G = EthernetSocket10G::getInstance();
        if (!eth10G) {
            LOG_TO_FILE("ERROR: Ethernet pointer is null.");
            return -1;
        }
        if(!eth10G->sendData(byArrPkt,pktLen,eth10G->RemoteIP.toStdString(),eth10G->Port)){
            LOG_TO_FILE("Sent filed!!!<eth10G>");
        }
        {
            char ByteArr64BitPakt[64]={0};
            std::string senderIp;
            uint16_t senderport;
            if(eth10G->receiveData(ByteArr64BitPakt,pktLen,senderIp,senderport))
            {
                reg_val = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
                LOG_TO_FILE("REG_VAL:0x%08X",reg_val);
            }else{
                LOG_TO_FILE("Receive filed!!!<eth10G>");
            }
        }
    }
    break;
    default:
        LOG_TO_FILE("No valid interface selection");
    }
    delete byArrPkt;
    return reg_val;
    LOG_TO_FILE(" DeviceSetup::readRegisterValue() <EXIT>");
}

void Spectrum::on_strmnStrt_radioButton_clicked()
{
    {
        if(ui->DDC_DataradioButton->isChecked())
        {
            /// Device reset
            //unsigned int regData=0;
            if(ui->IOnly_radioButton->isChecked())
            {
                int dataSize = ui->DataSize_comboBox->currentIndex();
                switch (dataSize) {
                case 0:
                    handleRegisterWrite(iface::eETH10G,0x2F4,(unsigned int)ui->windowSize_lineEdit->text().toInt()/4);

                    break;
                case 1:
                    handleRegisterWrite(iface::eETH10G,0x2F4,(unsigned int)ui->windowSize_lineEdit->text().toInt()/2);

                    break;
                case 2:
                    handleRegisterWrite(iface::eETH10G,0x2F4,(unsigned int)ui->windowSize_lineEdit->text().toInt());
                                        break;
                default:
                    break;
                }
            }
            if(ui->IQInterleved_radioButton->isChecked())
            {
                int dataSize = ui->DataSize_comboBox->currentIndex();
                switch (dataSize) {
                case 0:
                    handleRegisterWrite(iface::eETH10G,0x2F4,(unsigned int)ui->windowSize_lineEdit->text().toInt()/2);
                    break;
                case 1:
                    handleRegisterWrite(iface::eETH10G,0x2F4,(unsigned int)ui->windowSize_lineEdit->text().toInt());
                    break;
                case 2:
                    handleRegisterWrite(iface::eETH10G,0x2F4,(unsigned int)ui->windowSize_lineEdit->text().toInt()*2);
                    break;
                default:
                    break;
                }
            }
            uint reg_val = readRegisterValue(iface::eETH10G,0x2F8);
            reg_val = BitUtils::setValueInBits19to12(reg_val,ui->chnl_comboBox->currentIndex());
            handleRegisterWrite(iface::eETH10G,0x2F8,reg_val);

            reg_val = readRegisterValue(iface::eETH10G,0x2F8);
            BitUtils::setBit(reg_val,0);
            handleRegisterWrite(iface::eETH10G,0x2F8,reg_val);
            reg_val = readRegisterValue(iface::eETH10G,0x2F8);
            BitUtils::clearBit(reg_val,0);
            handleRegisterWrite(iface::eETH10G,0x2F8,reg_val);

            // pObjConnectionModes->objMiddleAPI.RegWrite(LFT_HOST_CONNECTION_ETH,0,LFT_SPPU_DEV_FPGA,0x0C,0x3);
           // pObjConnectionModes->objMiddleAPI.RegWrite(LFT_HOST_CONNECTION_ETH,0,LFT_SPPU_DEV_FPGA,0x04,0x10);
            if(ui->IQInterleved_radioButton->isChecked())
                X_graphPlot->setAxisScale(QwtPlot::xBottom,0,ui->lineEdit_fs->text().toInt(),1);
            if(ui->IOnly_radioButton->isChecked())
                X_graphPlot->setAxisScale(QwtPlot::xBottom,0,ui->lineEdit_fs->text().toInt()/2,1);
            Fs = ui->lineEdit_fs->text().toInt();
            windowSize = ui->windowSize_lineEdit->text().toInt();
        }
        ui->DDC_DataradioButton->setDisabled(true);
        //ui->raw_radioButton->setDisabled(true);
        ui->IQInterleved_radioButton->setDisabled(true);
        ui->IOnly_radioButton->setDisabled(true);
    }
}


void Spectrum::on_autoRefreshOn_radioButton_clicked()
{
    if(ui->strmnStrt_radioButton->isChecked())
    {
        eth10G = EthernetSocket10G::getInstance();
        if(eth10G != nullptr)
        {
            plotTimer->setInterval(ui->refrestRate_lineEdit->text().toInt());
            plotTimer->start();
        }
        else
        {
            QMessageBox::critical(this,"Error","No Connection with FPGA 1G",QMessageBox::Ok);
            ui->autoRefreshOff_radioButton->setChecked(true);
        }
    }
    else
    {

        QMessageBox::critical(this,"Invalid Selection","Please Check Start",QMessageBox::Ok);
        ui->autoRefreshOff_radioButton->setChecked(true);
    }
}

void Spectrum::on_strmnStop_radioButton_clicked()
{
    setupTransferAgent->abortTransfer();
    ui->DDC_DataradioButton->setDisabled(false);
    //ui->raw_radioButton->setDisabled(false);
    ui->IQInterleved_radioButton->setDisabled(false);
    ui->IOnly_radioButton->setDisabled(false);
    if(ui->DDC_DataradioButton->isChecked())
    {
        uint reg_val = readRegisterValue(iface::eETH10G,0x2f8);
        reg_val = BitUtils::setBit(reg_val,1);
        handleRegisterWrite(iface::eETH10G,0x2f8,reg_val);

        reg_val = readRegisterValue(iface::eETH10G,0x2F8);
        BitUtils::clearBit(reg_val,1);
        handleRegisterWrite(iface::eETH10G,0x2F8,reg_val);

    }
    plotTimer->stop();
    ui->DDC_DataradioButton->setDisabled(false);
    //ui->raw_radioButton->setDisabled(false);
    ui->IQInterleved_radioButton->setDisabled(false);
    ui->IOnly_radioButton->setDisabled(false);

    ui->autoRefreshOff_radioButton->setChecked(true);
    ui->ChkBoxFFtShift->setChecked(false);
    on_ChkBoxFFtShift_clicked(false);
    setupTransferAgent->abortTransfer();
}


void Spectrum::on_ChkBoxFFtShift_clicked(bool checked)
{
    maxHold = checked;
    if(checked)
    {
        X_graphPlot->setAxisScale(QwtPlot::xBottom,-Fs/2,Fs/2,1);
        // X_graphPlot->replot();
    }
    else
    {
        if(ui->IQInterleved_radioButton->isChecked())
            X_graphPlot->setAxisScale(QwtPlot::xBottom,0,ui->lineEdit_fs->text().toInt(),1);
        if(ui->IOnly_radioButton->isChecked())
            X_graphPlot->setAxisScale(QwtPlot::xBottom,0,ui->lineEdit_fs->text().toInt()/2,1);
        // X_graphPlot->replot();
    }
}


void Spectrum::on_ChkBoxMixerData_checkStateChanged(const Qt::CheckState &arg1)
{
    if(arg1 == Qt::Checked){
        handleRegisterWrite(iface::eETH10G,0x14,0x2);
    }
    else{
        handleRegisterWrite(iface::eETH10G,0x14,0x0);
    }
}


void Spectrum::on_ChkBoxADCData_checkStateChanged(const Qt::CheckState &arg1)
{
    if(arg1 == Qt::Checked){
        handleRegisterWrite(iface::eETH10G,0x14,0x1);
    }
    else{
        handleRegisterWrite(iface::eETH10G,0x14,0x0);
    }
}


void Spectrum::on_ChkBoxCICData_checkStateChanged(const Qt::CheckState &arg1)
{
    if(arg1 == Qt::Checked){
        handleRegisterWrite(iface::eETH10G,0x14,0x4);
    }
    else{
        handleRegisterWrite(iface::eETH10G,0x14,0x0);
    }
}


void Spectrum::on_ChkBoxCFIRData_checkStateChanged(const Qt::CheckState &arg1)
{
    if(arg1 == Qt::Checked){
        handleRegisterWrite(iface::eETH10G,0x14,0x8);
    }
    else{
        handleRegisterWrite(iface::eETH10G,0x14,0x0);
    }
}


void Spectrum::on_ChkBoxPFIRData_checkStateChanged(const Qt::CheckState &arg1)
{
    if(arg1 == Qt::Checked){
        handleRegisterWrite(iface::eETH10G,0x14,0x10);
    }
    else{
        handleRegisterWrite(iface::eETH10G,0x14,0x0);
    }
}


void Spectrum::on_DataSize_comboBox_currentIndexChanged(int index)
{
    if(ui->DataSize_comboBox->currentIndex() == 1){
         handleRegisterWrite(iface::eETH10G,0x304,0x1);
    }
    else{
        handleRegisterWrite(iface::eETH10G,0x304,0x0);
    }
}



void Spectrum::on_ChkBoxFFtShift_checkStateChanged(const Qt::CheckState &arg1)
{

}


void Spectrum::on_ChkBoxFFtShift_clicked()
{

}

