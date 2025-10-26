#include "spectrum.h"
#include "ui_spectrum.h"
#include "connectionctx.h"
#include <QVector>
#include <cmath>
#include <vector>
#include <QApplication>
#include <QVector>
#include <cmath>
#include <vector>

#include <QApplication>
#include <QVector>
#include <cmath>
#include <vector>
#include <fstream>
#include <iostream>
#include <qwt_plot_grid.h>
#include <log.h>
#include <QTimer>
#include <QtGlobal>
#include <QRandomGenerator>
#include <qwt_plot_canvas.h>

using namespace std;
Spectrum::Spectrum(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Spectrum)
{
    ui->setupUi(this);
    N = NUM_POINT;
    frq = FRQ;

    // Labels: numbers in radar-green, units in soft light text
    ui->frequency_label->setStyleSheet("color:#27D07D; font: 700 18pt \"Segoe UI\";");
    ui->frequency_label_db->setStyleSheet("color:#27D07D; font: 700 18pt \"Segoe UI\";");
    ui->label_mhz->setStyleSheet("color:#E6F8F1; font: 600 14pt \"Segoe UI\";");
    ui->label_db->setStyleSheet("color:#E6F8F1; font: 600 14pt \"Segoe UI\";");

    // Plot
    X_graphPlot = new QwtPlot(ui->spectrum_plot_frame);

    // Apply radar plot theme (canvas, axes, grid, fonts, margins)
    applyRadarPlotTheme(X_graphPlot);

    // If you want different y-range, keep your original line:
    // X_graphPlot->setAxisScale(QwtPlot::yLeft,-150,10,10);

    // Picker (crosshair/track) — use radar green instead of red
    picker = new QwtPlotPicker(
        X_graphPlot->xBottom,
        X_graphPlot->yLeft,
        QwtPicker::NoRubberBand,
        QwtPicker::AlwaysOn,
        X_graphPlot->canvas());

    picker->setRubberBandPen(QPen(RadarTheme::AccentGreenBright, 1));
    picker->setTrackerPen(QPen(RadarTheme::AccentGreenBright, 1));

    QFont trackerFont("Segoe UI");
    trackerFont.setBold(true);
    trackerFont.setPointSizeF(10);
    picker->setTrackerFont(trackerFont);

    pickerMachine = new QwtPickerDragPointMachine();
    picker->setStateMachine(pickerMachine);
    connect(picker, SIGNAL(moved(QPoint)), this, SLOT(plotPicker(QPoint)));

    // Layout
    auto *obj = new QHBoxLayout(this);
    ui->spectrum_plot_frame->setLayout(obj);
    obj->addWidget(X_graphPlot);

    FilePlay = false;

    // Curves
    curve_Y = new QwtPlotCurve();            // live spectrum
    curve_Y->setPen(QPen(RadarTheme::CurveMain, 2.0, Qt::SolidLine));
    curve_Y->attach(X_graphPlot);

    m_ObjMaxCurve = new QwtPlotCurve();      // max hold (if you use it)
    m_ObjMaxCurve->setPen(QPen(RadarTheme::CurveMaxHold, 1.5, Qt::DotLine));
    // m_ObjMaxCurve->attach(X_graphPlot); // attach when you actually use it

    X_graphPlot->show();

    // Timer, FFT buffers, and the rest unchanged
    plotTimer = new QTimer(this);
    connect(plotTimer, SIGNAL(timeout()), this, SLOT(on_plotTimer_TimeOut()));

    in  = (double*)         fftw_malloc(sizeof(double) * N);
    out = (fftw_complex*)   fftw_malloc(sizeof(fftw_complex) * N);

    ui->strmnStop_radioButton->setChecked(true);
    ui->autoRefreshOff_radioButton->setChecked(true);

    sample_count = 0;
    maxHold      = false;
    windowSize   = 2048;

    ui->pb_hide_show_menu->hide();
    m_vMaxHoldBuffer = NULL;

    // Keep your radio-button exclusivity logic
    ui->strmnStrt_radioButton->setAutoExclusive(false);
    ui->strmnStop_radioButton->setAutoExclusive(false);
    ui->DDC_DataradioButton->setAutoExclusive(false);
    ui->IQInterleved_radioButton->setAutoExclusive(false);
    ui->IOnly_radioButton->setAutoExclusive(false);

    ui->DDC_DataradioButton->setChecked(false);
    ui->IQInterleved_radioButton->setChecked(false);
    ui->IOnly_radioButton->setChecked(false);

    setupTransferAgent = new FileTransferAgent();
    connect(setupTransferAgent, &FileTransferAgent::transferComplete,this, &Spectrum::chunkReadCompleted);

    // Example: dynamic axis label color sync with theme (optional)
    // X_graphPlot->axisWidget(QwtPlot::xBottom)->setTitle(QwtText("MHz", QwtText::RichText));
    // X_graphPlot->axisWidget(QwtPlot::yLeft)->setTitle(QwtText("dBm", QwtText::RichText));
}

Spectrum::~Spectrum()
{
    delete ui;
}

// Reusable theming for a QwtPlot
void Spectrum::applyRadarPlotTheme(QwtPlot* plot)
{
    // Canvas background
    auto *canvas = qobject_cast<QwtPlotCanvas*>(plot->canvas());
    if (canvas) {
        QPalette pal = canvas->palette();
        pal.setColor(QPalette::Window, RadarTheme::CanvasDark);
        pal.setColor(QPalette::WindowText, RadarTheme::Text);
        canvas->setAutoFillBackground(true);
        canvas->setPalette(pal);
        canvas->setFrameStyle(QFrame::NoFrame);
    } else {
        plot->setCanvasBackground(RadarTheme::CanvasDark);
    }

    // Plot title (if any) & fonts
    QFont base("Segoe UI");
    base.setPointSize(10);
    plot->setTitle(QwtText()); // no title by default

    // Axis titles
    QwtText yTitle("dBm");
    yTitle.setFont(QFont("Segoe UI", 10, QFont::DemiBold));
    yTitle.setColor(RadarTheme::TextSoft);
    plot->setAxisTitle(QwtPlot::yLeft, yTitle);

    QwtText xTitle("MHz");
    xTitle.setFont(QFont("Segoe UI", 10, QFont::DemiBold));
    xTitle.setColor(RadarTheme::TextSoft);
    plot->setAxisTitle(QwtPlot::xBottom, xTitle);

    // Axis fonts & colors (ticks/labels)
    auto *xAxis = plot->axisWidget(QwtPlot::xBottom);
    auto *yAxis = plot->axisWidget(QwtPlot::yLeft);
    if (xAxis) {
        xAxis->setFont(QFont("Segoe UI", 9));
        QPalette p = xAxis->palette();
        p.setColor(QPalette::WindowText, RadarTheme::AxisTicks);
        p.setColor(QPalette::Text,       RadarTheme::AxisTicks);
        xAxis->setPalette(p);
        xAxis->setTitle(xTitle);
    }
    if (yAxis) {
        yAxis->setFont(QFont("Segoe UI", 9));
        QPalette p = yAxis->palette();
        p.setColor(QPalette::WindowText, RadarTheme::AxisTicks);
        p.setColor(QPalette::Text,       RadarTheme::AxisTicks);
        yAxis->setPalette(p);
        yAxis->setTitle(yTitle);
    }

    // Axis scales (keep your ranges—just here for clarity)
    plot->setAxisScale(QwtPlot::yLeft, -150, 10, 10);

    // Grid (subtle, radar-green)
    auto *grid = new QwtPlotGrid;
    grid->setPen(QPen(RadarTheme::Grid, 1, Qt::DotLine));
#if QWT_VERSION >= 0x060000
    grid->setMinorPen(QPen(RadarTheme::GridMinor, 1, Qt::DotLine));
    grid->enableXMin(true);
    grid->enableYMin(true);
#endif
    grid->attach(plot);

    // Margins around canvas so labels breathe
    plot->plotLayout()->setCanvasMargin(8, QwtPlot::xBottom);
    plot->plotLayout()->setAlignCanvasToScales(true);

    plot->replot();
}

void Spectrum::chunkReadCompleted()
{
    LOG_INFO("Transfer completed");
}

iface Spectrum::getSelectedDeviceType()
{
    auto& ctx = ConnectionHelper::instance();
    iface sel = ctx.selectedInterface();

    const QString ifaceName = Utils::ifaceToQString(sel);

    // 1️⃣ Check if no interface selected
    if (sel == eNONE)
    {
        LOG_ERROR("[Spectrum] No interface selected (iface=%s)", Utils::ifaceToCStr(sel));
        Log::showStatusMessage(this, "Device Setup", "Please select an interface before proceeding.");
        return eNONE;
    }

    // 2️⃣ Check if selected interface is connected
    ConnInfo info = ctx.info(sel);
    if (!info.connected)
    {
        LOG_ERROR("[Spectrum] Selected interface '%s' is NOT connected.", Utils::ifaceToCStr(sel));
        Log::showStatusMessage(this, "Device Setup",
                               QString("Selected interface '%1' is not connected.").arg(ifaceName));
        return eNONE;
    }

    // 3️⃣ Success — valid and connected interface
    LOG_INFO("[Spectrum] Selected and connected interface: %s", Utils::ifaceToCStr(sel));
    //Log::showStatusMessage(this, "Device Setup", QString("Selected Interface: %1").arg(ifaceName));

    return sel;
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
    LOG_INFO("Spectrum::on_pb_play_snap_shot_clicked ENTER");
    static int count =0;
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("Interface not selected");
        return;
    }

    if (deviceType == eNONE)
    {
        LOG_ERROR("No connection with FPGA 1G. Aborting snapshot.");
        QMessageBox::critical(this, "warning", "No Connection with FPGA 1G", QMessageBox::Ok);
        ui->pb_plot->setEnabled(false);
        if (plotTimer && plotTimer->isActive()) {
            plotTimer->stop();
            LOG_INFO("plotTimer stopped due to missing connection");
        }
        LOG_INFO("Spectrum::on_pb_play_snap_shot_clicked EXIT (no connection)");
        return;
    }

    LOG_INFO("Connection present and device selected. DeviceType: %d", static_cast<int>(deviceType));

    // Only handle DDC Data mode path here
    if (!ui->DDC_DataradioButton->isChecked()) {
        LOG_DEBUG("DDC Data radio button not checked; no streaming action taken");
        ui->pb_plot->setEnabled(true);
        LOG_INFO("Plot button enabled");
        LOG_INFO("Spectrum::on_pb_play_snap_shot_clicked EXIT");
        return;
    }

    LOG_INFO("DDC Data mode selected");

    // Determine which sub-mode and compute bytes to read
    if (ui->IOnly_radioButton->isChecked()) {
        LOG_INFO("I-only mode selected");
        unsigned int byteToRd = 0;
        int dataWidth = ui->DataSize_comboBox->currentIndex();
        LOG_DEBUG("Data width index: %d", dataWidth);

        switch (dataWidth) {
        case 0: byteToRd = windowSize * 2; break;
        case 1: byteToRd = windowSize * 4; break;
        case 2: byteToRd = windowSize * 8; break;
        default:
            LOG_ERROR("Unknown data width index: %d", dataWidth);
            ui->warning_label->setText("Invalid data width selection");
            ui->pb_plot->setEnabled(false);
            LOG_INFO("Spectrum::on_pb_play_snap_shot_clicked EXIT (invalid data width)");
            return;
        }

        LOG_DEBUG("Bytes to read: %u", byteToRd);
        unsigned int fifo_level = readRegisterValue(deviceType, 0x52C);
        fifo_level = BitUtils::extractBits15to0(fifo_level);
        LOG_DEBUG("Read Data Available: %u", fifo_level);

        if (fifo_level >= byteToRd) {
            QString filename = "StreamingData.bin";
            LOG_INFO("Sufficient FIFO level (%u >= %u). Starting FileReadWriteSetup with file: %s",
                     fifo_level, byteToRd, filename.toStdString().c_str());
            FileReadWriteSetup(deviceType, byteToRd, filename.toStdString().c_str(), eStream);
        } else {
            LOG_INFO("FIFO too low (%u < %u). Refresh rate may be too fast.", fifo_level, byteToRd);
            ui->warning_label->setText("FIFO Empty. Slow the Refresh Rate");
            ui->pb_plot->setEnabled(false);
            LOG_INFO("Spectrum::on_pb_play_snap_shot_clicked EXIT (fifo low)");
            return;
        }
    }
    else if (ui->IQInterleved_radioButton->isChecked()) {
        LOG_INFO("IQ Interleaved mode selected");
        unsigned int byteToRd = 0;
        int dataWidth = ui->DataSize_comboBox->currentIndex();
        LOG_DEBUG("Data width index: %d", dataWidth);

        switch (dataWidth) {
        case 0: byteToRd = windowSize * 4; break;
        case 1: byteToRd = windowSize * 8; break;
        case 2: byteToRd = windowSize * 16; break;
        default:
            LOG_ERROR("Unknown data width index (IQ): %d", dataWidth);
            ui->warning_label->setText("Invalid data width selection");
            ui->pb_plot->setEnabled(false);
            LOG_INFO("Spectrum::on_pb_play_snap_shot_clicked EXIT (invalid data width)");
            return;
        }

        LOG_DEBUG("Bytes to read: %u", byteToRd);
        unsigned int fifo_level = readRegisterValue(deviceType, 0x52C);
        fifo_level = BitUtils::extractBits15to0(fifo_level);
        LOG_DEBUG("Read Data Available: %u", fifo_level);

        if (fifo_level >= byteToRd) {
            QString filename = QStringLiteral("StreamingData%1.bin").arg(count++);
            //QString filename = "StreamingData.bin";
            LOG_INFO("Sufficient FIFO level (%u >= %u). Starting FileReadWriteSetup with file: %s",
                     fifo_level, byteToRd, filename.toStdString().c_str());
            FileReadWriteSetup(deviceType, byteToRd, filename.toStdString().c_str(), eStream);
        } else {
            LOG_INFO("FIFO too low (%u < %u). Refresh rate may be too fast.", fifo_level, byteToRd);
            ui->warning_label->setText("FIFO Empty. Slow the Refresh Rate");
            ui->pb_plot->setEnabled(false);
            LOG_INFO("Spectrum::on_pb_play_snap_shot_clicked EXIT (fifo low)");
            return;
        }
    }
    else {
        LOG_DEBUG("No DDC data sub-mode selected");
        ui->pb_plot->setEnabled(false);
        LOG_INFO("Spectrum::on_pb_play_snap_shot_clicked EXIT (no sub-mode)");
        return;
    }

    ui->pb_plot->setEnabled(true);
    LOG_INFO("Plot button enabled");
    LOG_INFO("Spectrum::on_pb_play_snap_shot_clicked EXIT");
    //exit(0);
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
#if 0
//fft shift test code
void Spectrum::FFT_Plot(int windowSize, fftw_complex* signal, fftw_complex* outBuffer)
{
    LOG_INFO("Spectrum::FFT_Plot(windowSize=%d) <ENTER>", windowSize);

    Fs = ui->lineEdit_fs->text().toDouble(); // Sampling frequency

    try {
        // Allocate memory
        std::vector<double> FF(windowSize);    // Frequency axis
        std::vector<double> mag(windowSize);   // Magnitude (linear)
        std::vector<double> db(windowSize);    // Magnitude (dB)

        // Create FFT plan (reuse if already exists)
        plan_forward = fftw_plan_dft_1d(windowSize, signal, outBuffer, FFTW_FORWARD, FFTW_ESTIMATE);
        if (!plan_forward) {
            LOG_ERROR("FFTW plan creation failed");
            return;
        }

        // Execute FFT
        fftw_execute(plan_forward);

        // --- Step 1: Compute Magnitude ---
        for (int i = 0; i < windowSize; ++i)
            mag[i] = std::sqrt(outBuffer[i][0] * outBuffer[i][0] + outBuffer[i][1] * outBuffer[i][1]);

        // --- Step 2: Apply FFT shift if enabled ---
        if (ui->ChkBoxFFtShift->isChecked()) {
            int half = windowSize / 2;
            for (int i = 0; i < half; ++i)
                std::swap(mag[i], mag[i + half]);
        }

        // --- Step 3: Build Frequency Axis ---
        const double df = Fs / windowSize;
        if (ui->ChkBoxFFtShift->isChecked()) {
            // Shifted axis: centered around 0 Hz
            for (int k = 0; k < windowSize; ++k)
                FF[k] = (k - windowSize / 2.0) * df;
        } else {
            // Normal axis: 0 → Fs
            for (int k = 0; k < windowSize; ++k)
                FF[k] = k * df;
        }

        // --- Step 4: Normalize and Convert to dB ---
        double max_val = *std::max_element(mag.begin(), mag.end());
        if (ui->enableWeight_checkBox->isChecked()) {
            max_val = ui->enableWeight_lineEdit->text().toDouble();
        }
        if (max_val <= 0) max_val = 1e-9;

        for (int i = 0; i < windowSize; ++i) {
            double val = mag[i] / max_val;
            db[i] = 20.0 * log10(std::max(val, 1e-12));
        }

        // --- Step 5: Plot ---
        if (ui->IOnly_radioButton->isChecked()) {
            curve_Y->setSamples(FF.data(), db.data(), windowSize / 2);
            curve_Y->setPen(Qt::yellow, 1.5, Qt::SolidLine);
        } else {
            curve_Y->setSamples(FF.data(), db.data(), windowSize);
            curve_Y->setPen(Qt::yellow, 1.5, Qt::SolidLine);
        }

        X_graphPlot->replot();

        // --- Step 6: Peak Frequency and Amplitude ---
        auto maxIt = std::max_element(db.begin(), db.end());
        int maxIndex = std::distance(db.begin(), maxIt);
        double max_db = *maxIt;
        double freqAtPeak = FF[maxIndex];

        ui->frequency_label->setText(QString::number(freqAtPeak, 'f', 2));
        ui->frequency_label_db->setText(QString::number(max_db, 'f', 2));

        LOG_INFO("Peak at %.2f Hz, %.2f dB", freqAtPeak, max_db);

        // Cleanup
        fftw_destroy_plan(plan_forward);
        LOG_INFO("FFT_Plot() <EXIT>");
    }
    catch (std::exception &e) {
        LOG_ERROR("Exception in FFT_Plot: %s", e.what());
    }
}
#endif
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

void Spectrum::FFT_Plot_Complex(int windowSize, fftw_complex* signal, fftw_complex* outBuffer)
{
    LOG_INFO("Spectrum::FFT_Plot_Complex(windowSize=%d) <ENTER>", windowSize);

    double Fs = ui->lineEdit_fs->text().toDouble();
    const double df = Fs / windowSize;

    fftw_plan plan_forward = fftw_plan_dft_1d(windowSize, signal, outBuffer, FFTW_FORWARD, FFTW_ESTIMATE);
    if (!plan_forward) {
        LOG_ERROR("FFTW plan creation failed (complex input).");
        return;
    }

    fftw_execute(plan_forward);

    // Compute magnitude
    std::vector<double> mag(windowSize);
    for (int i = 0; i < windowSize; ++i)
        mag[i] = std::sqrt(outBuffer[i][0]*outBuffer[i][0] + outBuffer[i][1]*outBuffer[i][1]);

    // Optional FFT shift (centers DC in middle)
    if (ui->ChkBoxFFtShift->isChecked()) {
        int half = windowSize / 2;
        for (int i = 0; i < half; ++i)
            std::swap(mag[i], mag[i + half]);
    }

    // Build frequency axis
    std::vector<double> freq(windowSize);
    if (ui->ChkBoxFFtShift->isChecked()) {
        for (int k = 0; k < windowSize; ++k)
            freq[k] = (k - windowSize / 2.0) * df;   // -Fs/2 → +Fs/2
    } else {
        for (int k = 0; k < windowSize; ++k)
            freq[k] = k * df;                        // 0 → Fs
    }

    // Normalize and convert to dB
    double max_val = *std::max_element(mag.begin(), mag.end());
    if (max_val <= 0) max_val = 1e-9;

    std::vector<double> db(windowSize);
    for (int i = 0; i < windowSize; ++i)
        db[i] = 20.0 * log10(std::max(mag[i] / max_val, 1e-12));

    // Plot
    curve_Y->setSamples(freq.data(), db.data(), windowSize);
    curve_Y->setPen(Qt::yellow, 1.5, Qt::SolidLine);
    curve_Y->attach(X_graphPlot);
    X_graphPlot->replot();

    // Find and display peak
    int maxIndex = std::max_element(db.begin(), db.end()) - db.begin();
    ui->frequency_label->setText(QString::number(freq[maxIndex], 'f', 2));
    ui->frequency_label_db->setText(QString::number(db[maxIndex], 'f', 2));

    fftw_destroy_plan(plan_forward);
    LOG_INFO("Spectrum::FFT_Plot_Complex() <EXIT>");
}

void Spectrum::FFT_Plot_Real(int windowSize, double* sample, fftw_complex* outBuffer)
{
    LOG_INFO("Spectrum::FFT_Plot_Real(windowSize=%d) <ENTER>", windowSize);

    double Fs = ui->lineEdit_fs->text().toDouble();
    const double df = Fs / windowSize;

    // Create FFT plan for real input
    fftw_plan plan_forward = fftw_plan_dft_r2c_1d(windowSize, sample, outBuffer, FFTW_ESTIMATE);
    if (!plan_forward) {
        LOG_ERROR("FFTW plan creation failed (real input).");
        return;
    }

    fftw_execute(plan_forward);

    // Magnitude (only N/2 valid bins for real FFT)
    int Nhalf = windowSize / 2;
    std::vector<double> mag(Nhalf);
    std::vector<double> freq(Nhalf);

    for (int i = 0; i < Nhalf; ++i) {
        mag[i] = std::sqrt(outBuffer[i][0]*outBuffer[i][0] + outBuffer[i][1]*outBuffer[i][1]);
        freq[i] = i * df;
    }

    // Normalize and convert to dB
    double max_val = *std::max_element(mag.begin(), mag.end());
    if (max_val <= 0) max_val = 1e-9;

    std::vector<double> db(Nhalf);
    for (int i = 0; i < Nhalf; ++i)
        db[i] = 20.0 * log10(std::max(mag[i] / max_val, 1e-12));

    // Plot (0 to Fs/2)
    curve_Y->setSamples(freq.data(), db.data(), Nhalf);
    curve_Y->setPen(Qt::yellow, 1.5, Qt::SolidLine);
    curve_Y->attach(X_graphPlot);
    X_graphPlot->replot();

    // Peak frequency
    int maxIndex = std::max_element(db.begin(), db.end()) - db.begin();
    ui->frequency_label->setText(QString::number(freq[maxIndex], 'f', 2));
    ui->frequency_label_db->setText(QString::number(db[maxIndex], 'f', 2));

    fftw_destroy_plan(plan_forward);
    LOG_INFO("Spectrum::FFT_Plot_Real() <EXIT>");
}

void Spectrum::on_pb_plot_with_file_clicked()
{
    LOG_INFO("Spectrum::on_pb_plot_with_file_clicked ENTER");

    if (ui->strmnStrt_radioButton->isChecked() == false) {
        LOG_ERROR("Start option not selected");
        QMessageBox::critical(this, "Not A Valid Setting", "Please select Start Option", QMessageBox::Ok);
        LOG_INFO("Spectrum::on_pb_plot_with_file_clicked EXIT (no start option)");
        return;
    }

    QString filter = "File Description (*.bin)";
    QString file_Name = QFileDialog::getOpenFileName(this, "Select a file...", QDir::currentPath(), filter);
    if (file_Name.isEmpty()) {
        LOG_INFO("No file selected by user");
        LOG_INFO("Spectrum::on_pb_plot_with_file_clicked EXIT (no file)");
        return;
    }

    FILE *fp = nullptr;
    fp = fopen(file_Name.toStdString().c_str(), "rb");
    if (!fp) {
        LOG_ERROR("Failed to open file: %s", file_Name.toStdString().c_str());
        LOG_INFO("Spectrum::on_pb_plot_with_file_clicked EXIT (file open failed)");
        return;
    }
    LOG_INFO("Opened file: %s", file_Name.toStdString().c_str());

    // determine file size
    fseek(fp, 0, SEEK_END);
    dwFileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    LOG_INFO("File size: %lld bytes", (long long)dwFileSize);

    // allocate buffers
    char *read = nullptr;
    fftw_complex *signal = nullptr;
    fftw_complex *outBuffer = nullptr;

    try {
        read = new char[dwFileSize];
        signal = new fftw_complex[windowSize];
        outBuffer = new fftw_complex[windowSize];
    } catch (const std::bad_alloc &e) {
        LOG_ERROR("Memory allocation failed: %s", e.what());
        if (fp) fclose(fp);
        delete[] read;
        delete[] signal;
        delete[] outBuffer;
        LOG_INFO("Spectrum::on_pb_plot_with_file_clicked EXIT (alloc fail)");
        return;
    }

    size_t readCount = fread(read, 1, dwFileSize, fp);
    fclose(fp);
    LOG_INFO("Read %zu bytes from file", readCount);

    ofstream fc_debug_I;
    ofstream fc_debug_Q;
    fc_debug_I.open("I_Data.txt", std::ios_base::out | std::ofstream::trunc);
    fc_debug_Q.open("Q_Data.txt", std::ios_base::out | std::ofstream::trunc);
    if (!fc_debug_I.is_open() || !fc_debug_Q.is_open()) {
        LOG_ERROR("Failed to open debug output files");
        // continue — not fatal for plotting
    } else {
        LOG_INFO("Debug output files opened");
    }

    if (ui->strmnStrt_radioButton->isChecked()) {
        if (ui->DDC_DataradioButton->isChecked()) {
            LOG_INFO("DDC Data mode selected. WindowSize=%d, Fs=%f", windowSize, Fs);

            if (ui->IQInterleved_radioButton->isChecked()) {
                LOG_INFO("IQ Interleaved mode selected");
                // IQ 16
                if (ui->DataSize_comboBox->currentIndex() == 0) {
                    LOG_INFO("IQ data size: 16-bit");
                    for (int i = 0; i < windowSize; ++i) {
                        short temp, temp2;
                        double multiplier = 0.5 * (1 - cos(2 * M_PI * i / (windowSize - 1))); // Hanning Window
                        temp  = ((unsigned char)read[4 * i + 0] << 8) & 0xFF00;
                        temp |= (unsigned char)read[4 * i + 1];
                        temp2 = ((unsigned char)read[4 * i + 2] << 8) & 0xFF00;
                        temp2 |= (unsigned char)read[4 * i + 3];

                        signal[i][1] = (double)temp2 * multiplier;
                        signal[i][0] = (double)temp * multiplier;

                        fc_debug_I << std::hex << temp2 << std::endl;
                        fc_debug_Q << std::hex << temp << std::endl;
                    }
                }

                // IQ 32
                if (ui->DataSize_comboBox->currentIndex() == 1) {
                    LOG_INFO("IQ data size: 32-bit");
                    for (int i = 0; i < windowSize; ++i) {
                        long int temp = 0;
                        long int temp2 = 0;
                        double multiplier = 0.5 * (1 - cos(2 * M_PI * i / (windowSize - 1)));
                        temp  = ((unsigned char)read[8 * i + 0] << 24) & 0xFF000000;
                        temp |= ((unsigned char)read[8 * i + 1] << 16) & 0xFF0000;
                        temp |= ((unsigned char)read[8 * i + 2] << 8) & 0xFF00;
                        temp |= ((unsigned char)read[8 * i + 3]) & 0xFF;

                        temp2  = ((unsigned char)read[8 * i + 4] << 24) & 0xFF000000;
                        temp2 |= ((unsigned char)read[8 * i + 5] << 16) & 0xFF0000;
                        temp2 |= ((unsigned char)read[8 * i + 6] << 8) & 0xFF00;
                        temp2 |= ((unsigned char)read[8 * i + 7]) & 0xFF;

                        signal[i][0] = (double)temp2 * multiplier;
                        signal[i][1] = (double)temp * multiplier;

                        fc_debug_Q << temp << std::endl;
                        fc_debug_I << temp2 << std::endl;
                    }
                }

                // IQ 64
                if (ui->DataSize_comboBox->currentIndex() == 2) {
                    LOG_INFO("IQ data size: 64-bit");
                    for (int i = 0; i < windowSize; ++i) {
                        long long temp = 0;
                        long long temp2 = 0;
                        double multiplier = 0.5 * (1 - cos(2 * M_PI * i / (windowSize - 1)));
                        temp  = ((unsigned long long)(unsigned char)read[16 * i] << 56) & 0xFF00000000000000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[16 * i + 1] << 48) & 0xFF000000000000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[16 * i + 2] << 40) & 0xFF0000000000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[16 * i + 3] << 32) & 0xFF00000000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[16 * i + 4] << 24) & 0xFF000000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[16 * i + 5] << 16) & 0xFF0000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[16 * i + 6] << 8) & 0xFF00ULL;
                        temp |= ((unsigned long long)(unsigned char)read[16 * i + 7]);

                        temp2  = ((unsigned long long)(unsigned char)read[16 * i + 8] << 56) & 0xFF00000000000000ULL;
                        temp2 |= ((unsigned long long)(unsigned char)read[16 * i + 9] << 48) & 0xFF000000000000ULL;
                        temp2 |= ((unsigned long long)(unsigned char)read[16 * i + 10] << 40) & 0xFF0000000000ULL;
                        temp2 |= ((unsigned long long)(unsigned char)read[16 * i + 11] << 32) & 0xFF00000000ULL;
                        temp2 |= ((unsigned long long)(unsigned char)read[16 * i + 12] << 24) & 0xFF000000ULL;
                        temp2 |= ((unsigned long long)(unsigned char)read[16 * i + 13] << 16) & 0xFF0000ULL;
                        temp2 |= ((unsigned long long)(unsigned char)read[16 * i + 14] << 8) & 0xFF00ULL;
                        temp2 |= ((unsigned long long)(unsigned char)read[16 * i + 15]);

                        signal[i][0] = (double)temp2 * multiplier;
                        signal[i][1] = (double)temp * multiplier;

                        fc_debug_I << temp << std::endl;
                        fc_debug_Q << temp2 << std::endl;
                    }
                }
            }

            if (ui->IOnly_radioButton->isChecked()) {
                LOG_INFO("I-only mode selected");

                // I-only 16
                if (ui->DataSize_comboBox->currentIndex() == 0) {
                    LOG_INFO("I-only data size: 16-bit");
                    for (int i = 0; i < windowSize; ++i) {
                        short temp;
                        short temp2 = 0;
                        double multiplier = 0.5 * (1 - cos(2 * M_PI * i / (windowSize - 1)));
                        temp = ((unsigned char)read[2 * i + 1] << 8) & 0xFF00;
                        temp |= (unsigned char)read[2 * i];
                        signal[i][0] = (double)temp2 * multiplier;
                        signal[i][1] = (double)temp * multiplier;

                        fc_debug_I << temp << std::endl;
                        fc_debug_Q << temp2 << std::endl;
                    }
                }

                // I-only 32
                if (ui->DataSize_comboBox->currentIndex() == 1) {
                    LOG_INFO("I-only data size: 32-bit");
                    for (int i = 0; i < windowSize; ++i) {
                        qint32 temp;
                        qint32 temp2 = 0;
                        double multiplier = 0.5 * (1 - cos(2 * M_PI * i / (windowSize - 1)));
                        temp  = ((unsigned char)read[4 * i + 0] << 24) & 0xFF000000;
                        temp |= ((unsigned char)read[4 * i + 1] << 16) & 0xFF0000;
                        temp |= ((unsigned char)read[4 * i + 2] << 8) & 0xFF00;
                        temp |= ((unsigned char)read[4 * i + 3]);

                        signal[i][0] = (double)temp2 * multiplier;
                        signal[i][1] = (double)temp * multiplier;

                        fc_debug_I << temp << std::endl;
                        fc_debug_Q << temp2 << std::endl;
                    }
                }

                // I-only 64
                if (ui->DataSize_comboBox->currentIndex() == 2) {
                    LOG_INFO("I-only data size: 64-bit");
                    for (int i = 0; i < windowSize; ++i) {
                        long long temp;
                        long long temp2 = 0;
                        double multiplier = 0.5 * (1 - cos(2 * M_PI * i / (windowSize - 1)));
                        temp  = ((unsigned long long)(unsigned char)read[8 * i] << 56) & 0xFF00000000000000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[8 * i + 1] << 48) & 0xFF000000000000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[8 * i + 2] << 40) & 0xFF0000000000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[8 * i + 3] << 32) & 0xFF00000000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[8 * i + 4] << 24) & 0xFF000000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[8 * i + 5] << 16) & 0xFF0000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[8 * i + 6] << 8) & 0xFF00ULL;
                        temp |= ((unsigned long long)(unsigned char)read[8 * i + 7]);

                        signal[i][0] = (double)temp2 * multiplier;
                        signal[i][1] = (double)temp * multiplier;

                        fc_debug_I << temp << std::endl;
                        fc_debug_Q << temp2 << std::endl;
                    }
                }
            }
        }
    }

    LOG_INFO("Running FFT_Plot with windowSize=%d", windowSize);
    FFT_Plot(windowSize, signal, outBuffer);

    // cleanup
    delete[] read;
    delete[] signal;
    delete[] outBuffer;

    if (fc_debug_I.is_open()) fc_debug_I.close();
    if (fc_debug_Q.is_open()) fc_debug_Q.close();

    LOG_INFO("Spectrum::on_pb_plot_with_file_clicked EXIT");
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
                bool byteReversed =  0 ;//ui->byteReverse_checkBox->isChecked();
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
                bool byteReversed = 0;
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
    LOG_INFO("Spectrum::on_pb_plot_clicked ENTER");

    if (windowSize < 256 || windowSize > 8192) {
        LOG_ERROR("Invalid Window Size: %d (valid range 256..8192)", windowSize);
        QMessageBox::critical(this, "warning", "invalid Window SIze \n Out Of Range of 256 to 8192", QMessageBox::Ok);
        LOG_INFO("Spectrum::on_pb_plot_clicked EXIT (invalid window size)");
        return;
    }

    LOG_INFO("Plot enter. windowSize=%d", windowSize);

    FILE *fp = fopen("StreamingData.bin", "rb");
    if (fp == nullptr) {
        LOG_ERROR("Failed to open StreamingData.bin");
        LOG_INFO("Spectrum::on_pb_plot_clicked EXIT (file open failed)");
        return;
    }

    // determine file size
    fseek(fp, 0, SEEK_END);
    dwFileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    LOG_INFO("StreamingData.bin size = %lld bytes", (long long)dwFileSize);

    // allocate buffers, check for allocation failures
    char *read = nullptr;
    fftw_complex *signal = nullptr;
    fftw_complex *outBuffer = nullptr;

    read = new (std::nothrow) char[dwFileSize];
    if (!read) {
        LOG_ERROR("Memory allocation failed for read buffer (%lld bytes)", (long long)dwFileSize);
        fclose(fp);
        LOG_INFO("Spectrum::on_pb_plot_clicked EXIT (alloc fail)");
        return;
    }

    signal = new (std::nothrow) fftw_complex[windowSize];
    if (!signal) {
        LOG_ERROR("Memory allocation failed for signal (windowSize=%d)", windowSize);
        delete[] read;
        fclose(fp);
        LOG_INFO("Spectrum::on_pb_plot_clicked EXIT (alloc fail)");
        return;
    }

    outBuffer = new (std::nothrow) fftw_complex[windowSize];
    if (!outBuffer) {
        LOG_ERROR("Memory allocation failed for outBuffer (windowSize=%d)", windowSize);
        delete[] read;
        delete[] signal;
        fclose(fp);
        LOG_INFO("Spectrum::on_pb_plot_clicked EXIT (alloc fail)");
        return;
    }

    size_t bytesRead = fread(read, 1, dwFileSize, fp);
    fclose(fp);
    LOG_INFO("Read %zu bytes from StreamingData.bin", bytesRead);

    std::ofstream fc_debug_I;
    fc_debug_I.open("I_Data.txt", std::ios_base::out | std::ofstream::trunc);
    if (!fc_debug_I.is_open()) {
        LOG_ERROR("Failed to open I_Data.txt for writing");
    } else {
        LOG_INFO("Opened I_Data.txt");
    }

    std::ofstream fc_debug_Q;
    fc_debug_Q.open("Q_Data.txt", std::ios_base::out | std::ofstream::trunc);
    if (!fc_debug_Q.is_open()) {
        LOG_ERROR("Failed to open Q_Data.txt for writing");
    } else {
        LOG_INFO("Opened Q_Data.txt");
    }

    if (ui->strmnStrt_radioButton->isChecked()) {
        if (ui->DDC_DataradioButton->isChecked()) {
            LOG_INFO("DDC Data mode selected. windowSize=%d Fs=%f", windowSize, Fs);

            if (ui->IQInterleved_radioButton->isChecked()) {
                LOG_INFO("IQ interleaved path selected. data size index=%d", ui->DataSize_comboBox->currentIndex());

                // IQ 16
                if (ui->DataSize_comboBox->currentIndex() == 0) {
                    for (int i = 0; i < windowSize; ++i) {
                        short temp;
                        short temp2;
                        double multiplier = 0.5 * (1 - cos(2 * M_PI * i / (windowSize - 1))); // Hanning Window
                        temp  = ((unsigned char)read[4 * i] << 8) & 0xFF00;
                        temp |= (unsigned char)read[4 * i + 1];
                        temp2 = ((unsigned char)read[4 * i + 2] << 8) & 0xFF00;
                        temp2 |= (unsigned char)read[4 * i + 3];
                        signal[i][0] = (double)temp * multiplier;
                        signal[i][1] = (double)temp2 * multiplier;
                    }
                    LOG_INFO("Completed IQ16 unpack for windowSize=%d", windowSize);
                }

                // IQ 32
                if (ui->DataSize_comboBox->currentIndex() == 1) {
                    LOG_INFO("IQ interleaved 32-bit path");
                    for (int i = 0; i < windowSize; ++i) {
                        int temp = 0;
                        int temp2 = 0;
                        double multiplier = 0.5 * (1 - cos(2 * M_PI * i / (windowSize - 1)));
                        temp  = ((unsigned char)read[8 * i + 0] << 24) & 0xFF000000;
                        temp |= ((unsigned char)read[8 * i + 1] << 16) & 0xFF0000;
                        temp |= ((unsigned char)read[8 * i + 2] << 8) & 0xFF00;
                        temp |= ((unsigned char)read[8 * i + 3]);
                        temp2  = ((unsigned char)read[8 * i + 4] << 24) & 0xFF000000;
                        temp2 |= ((unsigned char)read[8 * i + 5] << 16) & 0xFF0000;
                        temp2 |= ((unsigned char)read[8 * i + 6] << 8) & 0xFF00;
                        temp2 |= ((unsigned char)read[8 * i + 7]);
                        signal[i][0] = (double)temp * multiplier;
                        signal[i][1] = (double)temp2 * multiplier;
                        fc_debug_I << std::hex << temp2 << std::endl;
                        fc_debug_Q << std::hex << temp << std::endl;
                    }
                    LOG_INFO("Completed IQ32 unpack for windowSize=%d", windowSize);
                }

                // IQ 64
                if (ui->DataSize_comboBox->currentIndex() == 2) {
                    LOG_INFO("IQ interleaved 64-bit path");
                    for (int i = 0; i < windowSize; ++i) {
                        long long temp = 0;
                        long long temp2 = 0;
                        double multiplier = 0.5 * (1 - cos(2 * M_PI * i / (windowSize - 1)));
                        temp  = ((unsigned long long)(unsigned char)read[16 * i] << 56) & 0xFF00000000000000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[16 * i + 1] << 48) & 0xFF000000000000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[16 * i + 2] << 40) & 0xFF0000000000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[16 * i + 3] << 32) & 0xFF00000000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[16 * i + 4] << 24) & 0xFF000000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[16 * i + 5] << 16) & 0xFF0000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[16 * i + 6] << 8) & 0xFF00ULL;
                        temp |= ((unsigned long long)(unsigned char)read[16 * i + 7]);
                        temp2  = ((unsigned long long)(unsigned char)read[16 * i + 8] << 56) & 0xFF00000000000000ULL;
                        temp2 |= ((unsigned long long)(unsigned char)read[16 * i + 9] << 48) & 0xFF000000000000ULL;
                        temp2 |= ((unsigned long long)(unsigned char)read[16 * i + 10] << 40) & 0xFF0000000000ULL;
                        temp2 |= ((unsigned long long)(unsigned char)read[16 * i + 11] << 32) & 0xFF00000000ULL;
                        temp2 |= ((unsigned long long)(unsigned char)read[16 * i + 12] << 24) & 0xFF000000ULL;
                        temp2 |= ((unsigned long long)(unsigned char)read[16 * i + 13] << 16) & 0xFF0000ULL;
                        temp2 |= ((unsigned long long)(unsigned char)read[16 * i + 14] << 8) & 0xFF00ULL;
                        temp2 |= ((unsigned long long)(unsigned char)read[16 * i + 15]);
                        signal[i][0] = (double)temp2 * multiplier;
                        signal[i][1] = (double)temp * multiplier;
                    }
                    LOG_INFO("Completed IQ64 unpack for windowSize=%d", windowSize);
                }
            } // end IQInterleved

            if (ui->IOnly_radioButton->isChecked()) {
                LOG_INFO("I-only path selected. data size index=%d", ui->DataSize_comboBox->currentIndex());

                // I-only 16
                if (ui->DataSize_comboBox->currentIndex() == 0) {
                    LOG_INFO("I-only 16-bit unpack");
                    for (int i = 0; i < windowSize; ++i) {
                        short temp;
                        short temp2 = 0;
                        double multiplier = 0.5 * (1 - cos(2 * M_PI * i / (windowSize - 1)));
                        temp = ((unsigned char)read[2 * i] << 8) & 0xFF00;
                        temp |= (unsigned char)read[2 * i + 1];
                        signal[i][0] = (double)temp2 * multiplier;
                        signal[i][1] = (double)temp * multiplier;
                    }
                }

                // I-only 32
                if (ui->DataSize_comboBox->currentIndex() == 1) {
                    LOG_INFO("I-only 32-bit unpack");
                    for (int i = 0; i < windowSize; ++i) {
                        int temp = 0;
                        int temp2 = 0;
                        double multiplier = 0.5 * (1 - cos(2 * M_PI * i / (windowSize - 1)));
                        temp  = ((unsigned char)read[4 * i] << 24) & 0xFF000000;
                        temp |= ((unsigned char)read[4 * i + 1] << 16) & 0xFF0000;
                        temp |= ((unsigned char)read[4 * i + 2] << 8) & 0xFF00;
                        temp |= ((unsigned char)read[4 * i + 3]);
                        signal[i][0] = (double)temp2 * multiplier;
                        signal[i][1] = (double)temp * multiplier;
                    }
                }

                // I-only 64
                if (ui->DataSize_comboBox->currentIndex() == 2) {
                    LOG_INFO("I-only 64-bit unpack");
                    for (int i = 0; i < windowSize; ++i) {
                        long long temp = 0;
                        long long temp2 = 0;
                        double multiplier = 0.5 * (1 - cos(2 * M_PI * i / (windowSize - 1)));
                        temp  = ((unsigned long long)(unsigned char)read[8 * i] << 56) & 0xFF00000000000000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[8 * i + 1] << 48) & 0xFF000000000000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[8 * i + 2] << 40) & 0xFF0000000000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[8 * i + 3] << 32) & 0xFF00000000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[8 * i + 4] << 24) & 0xFF000000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[8 * i + 5] << 16) & 0xFF0000ULL;
                        temp |= ((unsigned long long)(unsigned char)read[8 * i + 6] << 8) & 0xFF00ULL;
                        temp |= ((unsigned long long)(unsigned char)read[8 * i + 7]);
                        signal[i][0] = (double)temp2 * multiplier;
                        signal[i][1] = (double)temp * multiplier;
                    }
                }
            } // end IOnly
        } // end DDC_DataradioButton
    } // end strmnStrt_radioButton

    LOG_INFO("Calling FFT_Plot with windowSize=%d", windowSize);
    FFT_Plot(windowSize, signal, outBuffer);

    // cleanup
    if (fc_debug_I.is_open()) fc_debug_I.close();
    if (fc_debug_Q.is_open()) fc_debug_Q.close();
    delete[] read;
    delete[] signal;
    delete[] outBuffer;

    LOG_INFO("Spectrum::on_pb_plot_clicked EXIT");
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
    case iface::eETHPL1G:
    {
        stFileReadWriteConf Cnf;
        Cnf.iFileSize = iFileSize;
        Cnf.sFilePath = sFilePath;
        Cnf.eInterface = deviceType;
        Cnf._Dir = dir;
        LOG_INFO("eETHPL1G: iFileSize:%d,sFilePath:%s",iFileSize,sFilePath.toStdString().c_str());
        setupTransferAgent->configure(Cnf);
        setupTransferAgent->start();

    }
    break;
    case iface::ePCIe:
        break;
    case iface::eETH10G:
    {
        stFileReadWriteConf Cnf;
        Cnf.iFileSize = iFileSize;
        Cnf.sFilePath = sFilePath;
        Cnf.eInterface = deviceType;
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
    LOG_INFO("Spectrum::handleRegisterWrite <ENTER> Addr:0x%08X, value:0x%08X", iaddr, ival);
    char* byArrPkt = nullptr;

    Proto protocolobj;
    int pktLen = protocolobj.mPktRegWrite(iaddr, ival, &byArrPkt);
    switch (deviceType)
    {
    case iface::eSERIAL:
    {
        serial = UartSerial::getInstance();
        if (!serial) {
            LOG_ERROR("ERROR: Serial pointer is null.");
            if (byArrPkt) delete[] byArrPkt;
            LOG_INFO("Spectrum::handleRegisterWrite EXIT (serial null)");
            return;
        }
        if(!serial->sendData(byArrPkt, pktLen)){
            LOG_ERROR("Serial sendData failed");
        }
        break;
    }
    case iface::eETHPL1G:
    {
        Utils::RegWrite(deviceType,iaddr,ival);
        break;
    }
    case eETH10G:
        eth10G = EthernetSocket10G::getInstance();
        if (!eth10G) {
            LOG_ERROR("ERROR: Ethernet pointer is null.");
            if (byArrPkt) delete[] byArrPkt;
            LOG_INFO("Spectrum::handleRegisterWrite EXIT (eth10G null)");
            return;
        }
        if(!eth10G->sendData(byArrPkt,pktLen,eth10G->RemoteIP.toStdString(),eth10G->Port)){
            LOG_ERROR("eth10G sendData failed");
        }
        {
            char ByteArr64BitPakt[64]={0};
            std::string senderIp;
            uint16_t senderport;
            // Read and discard the packet
            if(eth10G->receiveData(ByteArr64BitPakt,pktLen,senderIp,senderport))
            {
                int reg_val = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
                LOG_INFO("RegVal:0x%08X", reg_val);
            } else {
                LOG_ERROR("eth10G receiveData failed");
            }
        }
        break;
    case ePCIe:
        break;
    case eNONE:
        break;
    case eETHPS1G:
        break;
    case ePLSERIAL:
        break;
    }

    if (byArrPkt) {
        delete[] byArrPkt;
        byArrPkt = nullptr;
    }

    LOG_INFO("Spectrum::handleRegisterWrite EXIT");
}

uint Spectrum::readRegisterValue(iface deviceType, uint addr)
{
    LOG_INFO("Spectrum::readRegisterValue ENTER. deviceType=%d addr=0x%08X", static_cast<int>(deviceType), addr);

    char *byArrPkt = nullptr;
    uint reg_val = 0;
    char ByteArr64BitPakt[64] = {0};
    Proto protocolobj;
    int pktLen = protocolobj.mPktRegRead(addr, &byArrPkt);

    switch (deviceType)
    {
    case iface::eSERIAL:
    {
        serial = UartSerial::getInstance();
        if (!serial) {
            LOG_ERROR("Serial pointer is null");
            if (byArrPkt) delete[] byArrPkt;
            LOG_INFO("DeviceSetup::readRegisterValue EXIT (serial null)");
            return static_cast<uint>(-1);
        }

        if (!serial->sendData(byArrPkt, pktLen)) {
            LOG_ERROR("Serial sendData failed");
        } else {
            // second send appears in original code; attempt receive once
            if (serial->receiveData(ByteArr64BitPakt, pktLen)) {
                int parsed = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
                reg_val = static_cast<uint>(parsed);
                LOG_INFO("REG_VAL: 0x%08X (serial)", reg_val);
            } else {
                LOG_ERROR("Serial receiveData failed");
            }
        }
        break;
    }
    case iface::eETHPL1G:
    {
        reg_val = Utils::RegRead(deviceType, addr);
        break;
    }
    case iface::ePCIe:
    {
        LOG_ERROR("PCIe path not implemented");
        break;
    }
    case iface::eETH10G:
    {
        reg_val = Utils::RegRead(deviceType, addr);
        break;
    }
    default:
        LOG_ERROR("No valid interface selection");
        break;
    }

    if (byArrPkt) {
        delete[] byArrPkt;
        byArrPkt = nullptr;
    }
    LOG_INFO("Spectrum::readRegisterValue EXIT. addr=0x%08X reg_val=0x%08X", addr, reg_val);
    return reg_val;
}

void Spectrum::on_strmnStrt_radioButton_clicked()
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("[WriteRegister] Interface not selected");
        return;
    }
    if(ui->DDC_DataradioButton->isChecked())
    {
        /// Device reset
        //unsigned int regData=0;
        if(ui->IOnly_radioButton->isChecked())
        {
            int dataSize = ui->DataSize_comboBox->currentIndex();
            switch (dataSize) {
            case 0:
                handleRegisterWrite(deviceType,0x51C,(unsigned int)ui->windowSize_lineEdit->text().toInt()/4);

                break;
            case 1:
                handleRegisterWrite(deviceType,0x51C,(unsigned int)ui->windowSize_lineEdit->text().toInt()/2);

                break;
            case 2:
                handleRegisterWrite(deviceType,0x51C,(unsigned int)ui->windowSize_lineEdit->text().toInt());
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
                handleRegisterWrite(deviceType,0x51C,(unsigned int)ui->windowSize_lineEdit->text().toInt()/2);
                break;
            case 1:
                handleRegisterWrite(deviceType,0x51C,(unsigned int)ui->windowSize_lineEdit->text().toInt());
                break;
            case 2:
                handleRegisterWrite(deviceType,0x51C,(unsigned int)ui->windowSize_lineEdit->text().toInt()*2);
                break;
            default:
                break;
            }
        }
        uint reg_val = readRegisterValue(deviceType,0x520);
        reg_val = BitUtils::setValueInBits19to12(reg_val,ui->chnl_comboBox->currentIndex());
        handleRegisterWrite(deviceType,0x520,reg_val);

        reg_val = readRegisterValue(deviceType,0x520);
        BitUtils::setBit(reg_val,0);
        handleRegisterWrite(deviceType,0x520,reg_val);
        reg_val = readRegisterValue(deviceType,0x520);
        BitUtils::clearBit(reg_val,0);
        handleRegisterWrite(deviceType,0x520,reg_val);

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


void Spectrum::on_autoRefreshOn_radioButton_clicked()
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("[WriteRegister] Interface not selected");
        QMessageBox::critical(this, "Error", "Interface not selected", QMessageBox::Ok);
        ui->autoRefreshOff_radioButton->setChecked(true);
        return;
    }

    if (!ui->strmnStrt_radioButton->isChecked()) {
        QMessageBox::critical(this, "Invalid Selection", "Please Check Start", QMessageBox::Ok);
        ui->autoRefreshOff_radioButton->setChecked(true);
        return;
    }

    bool ok = false;
    int refreshRate = ui->refrestRate_lineEdit->text().toInt(&ok);
    if (!ok || refreshRate <= 0) {
        QMessageBox::critical(this, "Invalid Input", "Refresh rate must be a positive number", QMessageBox::Ok);
        ui->autoRefreshOff_radioButton->setChecked(true);
        return;
    }

    plotTimer->setInterval(refreshRate);
    plotTimer->start();
}

void Spectrum::enableDataModeRadioButtons(bool enable)
{
    ui->DDC_DataradioButton->setDisabled(!enable);
    // ui->raw_radioButton->setDisabled(!enable); // Uncomment if needed
    ui->IQInterleved_radioButton->setDisabled(!enable);
    ui->IOnly_radioButton->setDisabled(!enable);
}

void Spectrum::toggleDDCControlBit(iface deviceType, uint address, int bit)
{
    uint reg_val = readRegisterValue(deviceType, address);
    reg_val = BitUtils::setBit(reg_val, bit);
    handleRegisterWrite(deviceType, address, reg_val);

    reg_val = readRegisterValue(deviceType, address);
    reg_val = BitUtils::clearBit(reg_val, bit);
    handleRegisterWrite(deviceType, address, reg_val);
}

void Spectrum::on_strmnStop_radioButton_clicked()
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("Interface not selected");
        return;
    }

    // Stop ongoing data transfer and plotting
    setupTransferAgent->abortTransfer();
    plotTimer->stop();

    // Re-enable radio buttons
    enableDataModeRadioButtons(true);

    // Reset DDC register if DDC mode was active
    if (ui->DDC_DataradioButton->isChecked()) {
        toggleDDCControlBit(deviceType, 0x520, 1);
    }

    // Reset UI state
    ui->autoRefreshOff_radioButton->setChecked(true);
    ui->ChkBoxFFtShift->setChecked(false);
    on_ChkBoxFFtShift_clicked(false);
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
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("Interface not selected");
        return;
    }

    if(arg1 == Qt::Checked){
        handleRegisterWrite(deviceType,0x514,0x2);
    }
    else{
        handleRegisterWrite(deviceType,0x514,0x0);
    }
}


void Spectrum::on_ChkBoxADCData_checkStateChanged(const Qt::CheckState &arg1)
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("Interface not selected");
        return;
    }

    if(arg1 == Qt::Checked){
        handleRegisterWrite(deviceType,0x514,0x1);
    }
    else{
        handleRegisterWrite(deviceType,0x514,0x0);
    }
}


void Spectrum::on_ChkBoxCICData_checkStateChanged(const Qt::CheckState &arg1)
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("Interface not selected");
        return;
    }

    if(arg1 == Qt::Checked){
        handleRegisterWrite(deviceType,0x514,0x4);
    }
    else{
        handleRegisterWrite(deviceType,0x514,0x0);
    }
}


void Spectrum::on_ChkBoxCFIRData_checkStateChanged(const Qt::CheckState &arg1)
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("Interface not selected");
        return;
    }

    if(arg1 == Qt::Checked){
        handleRegisterWrite(deviceType,0x514,0x8);
    }
    else{
        handleRegisterWrite(deviceType,0x514,0x0);
    }
}


void Spectrum::on_ChkBoxPFIRData_checkStateChanged(const Qt::CheckState &arg1)
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        LOG_ERROR("Interface not selected");
        return;
    }

    if(arg1 == Qt::Checked){
        handleRegisterWrite(deviceType,0x514,0x10);
    }
    else{
        handleRegisterWrite(deviceType,0x514,0x0);
    }
}

void Spectrum::on_DataSize_comboBox_currentIndexChanged(int index)
{
    iface deviceType = getSelectedDeviceType();
    if (deviceType == eNONE) {
        Log::showStatusMessage(this, "Device Setup", "Please select an interface");
        LOG_ERROR("Interface not selected");
        return;
    }

    if(ui->DataSize_comboBox->currentIndex() == 1)
    {
         handleRegisterWrite(deviceType,0x528,0x1);
    }
    else{
        handleRegisterWrite(deviceType,0x528,0x0);
    }
}

void Spectrum::on_ChkBoxFFtShift_checkStateChanged(const Qt::CheckState &arg1)
{

}


void Spectrum::on_ChkBoxFFtShift_clicked()
{

}

