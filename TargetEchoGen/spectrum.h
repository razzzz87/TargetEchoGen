#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <QWidget>
#include "qwt_plot.h"
#include <qwt_plot_curve.h>
#include <qwt_point_data.h>
#include "qcustomplot.h"
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
#include <fftw3.h>
//#include <mapi_thread.h>
#include <ethernetsocket10G.h>
#include <ethernetsocket.h>
#include "FileTransferAgent.h"
#include "uartserial.h"
#include "RegDef.h"

//#include <qspectrogram.h>
//#include <spectrogram.h>
#define NUM_POINT 2048
#define FRQ 50
namespace Ui {
class Spectrum;
}

class Spectrum : public QWidget
{
    Q_OBJECT

public:
    explicit Spectrum(QWidget *parent = nullptr);
    ~Spectrum();
    void FFT();
    void resizeEvent(QResizeEvent *);
    void mouseMoveEvent(QMouseEvent *e);
    void FFT_Plot(int windowSize, double *sample, fftw_complex *outBuffer);
    void FFT_Plot(int windowSize, fftw_complex *signal, fftw_complex *outBuffer);
    void fftshift(void *data, int N, size_t elementSize);
    void plotest();
    void handleRegisterWrite(iface deviceType, uint iaddr, uint ival);
    uint readRegisterValue(iface deviceType,uint addr);
    void FileReadWriteSetup(iface deviceType, uint iFileSize, QString sFilePath, eXferDir dir);
    QwtPlot* X_graphPlot;
    QwtPlotCurve *curve_Y ;
    QwtPlotPicker *picker;
    QwtPickerMachine* pickerMachine;
    QMessageBox *stat_Message;
    //MAPI_Thread *obj_thread;
    QTimer *plotTimer;
    QTimer *playBack_Timer;

    EthernetSocket10G *eth10G;
    EthernetSocket *eth1G;
    UartSerial *serial;
    //singeltonConnectionMode *pObjConnectionModes;
    double *data;
    char* adcData_Point;
    long readDataFromFpGA();
    double Fs;
    int windowSize;
    int sample_count;
    unsigned long long dwFileSize;
    unsigned long long playBack_Counter;// vaiable use to see how many bytes is processed
    unsigned int RAWMode_windowSize;
    bool maxHold;
    std::vector<double> numbers;
    QList <double> fft_data;
    FILE *filePlayBack_fp ;
    bool FilePlay;
    void hide();

    double y;
    int N;// 2028 Number of points acquired inside the window
    double frq;//50 frequencyr
    double *in;
    fftw_complex *out;
    fftw_complex *out2;
    double t[NUM_POINT];//time vector
    double ff[NUM_POINT];
    fftw_plan plan_forward;
    fftw_plan plan_forward2;
    //
    QwtPlot *m_PlotSpectrum;
    QwtMatrixRasterData *m_SPectrumMatrix;
    QwtPlotSpectrogram *spectrogram1;

protected:

    inline short parse16(const unsigned char* ptr, bool reverse = false) {
        return reverse ? (ptr[1] << 8) | ptr[0] : (ptr[0] << 8) | ptr[1];
    }

    inline qint32 parse32(const unsigned char* ptr, bool reverse = false) {
        return reverse
                   ? (ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0]
                   : (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
    }
    inline long long parse64(const unsigned char* ptr, bool reverse = false) {
        long long val = 0;
        for (int b = 0; b < 8; ++b) {
            int shift = reverse ? 8 * b : 56 - 8 * b;
            val |= ((unsigned long long)ptr[b] << shift);
        }
        return val;
    }
public slots:
    void chunkReadCompleted();

private slots:
    void on_pb_hide_show_menu_clicked();
    void on_pb_play_snap_shot_clicked();
    void on_pb_plot_with_file_clicked();
    void on_m_PBPlayRecordData_clicked();
    void on_pb_plot_clicked();
    void on_m_PBGetADCData_clicked();
    void plotPicker(QPoint actualMousePosition);
    void playFile();
    void on_plotTimer_TimeOut();
    void thread_Finished();
    void cancelThread();
    void UpdateProgressBAR(qint64 RecvDataDone);
    void on_chnl_comboBox_currentIndexChanged(int index);
    void on_strmnStrt_radioButton_clicked();
    void on_autoRefreshOn_radioButton_clicked();
    void on_strmnStop_radioButton_clicked();
    void on_ChkBoxFFtShift_clicked(bool checked);
    void on_ChkBoxMixerData_checkStateChanged(const Qt::CheckState &arg1);
    void on_ChkBoxADCData_checkStateChanged(const Qt::CheckState &arg1);
    void on_ChkBoxCICData_checkStateChanged(const Qt::CheckState &arg1);
    void on_ChkBoxCFIRData_checkStateChanged(const Qt::CheckState &arg1);
    void on_ChkBoxPFIRData_checkStateChanged(const Qt::CheckState &arg1);
    void on_DataSize_comboBox_currentIndexChanged(int index);
    void on_ChkBoxFFtShift_checkStateChanged(const Qt::CheckState &arg1);
    void on_ChkBoxFFtShift_clicked();
private:
    Ui::Spectrum *ui;
    bool toggle_ctrl_btn;
    QwtPlotCurve *m_ObjMaxCurve ;
    bool m_bMaxHoldEnable;
    double *m_vMaxHoldBuffer;
    void CreateSpectrumWidget();
    FileTransferAgent *setupTransferAgent;
};

#endif // SPECTRUM_H
