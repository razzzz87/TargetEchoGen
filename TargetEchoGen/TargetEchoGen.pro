QT  += core gui
QT  += network
QT  += serialport
QT  += printsupport
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
win32:LIBS += -lws2_32
QMAKE_CXXFLAGS += -Wa,-mbig-obj
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Proto.cpp \
    connectiontype.cpp \
    devicesetup.cpp \
    ethernetsocket.cpp \
    ethernetsocket10G.cpp \
    fileprocessing.cpp \
    fileprocessingconf.cpp \
    FileTransferAgent.cpp \
    log.cpp \
    main.cpp \
    mainwindow.cpp \
    matfileprocessing.cpp \
    qcustomplot.cpp \
    qspectrogram.cpp \
    rf.cpp \
    selftest.cpp \
    serial_port_singletonPl.cpp \
    serial_port_singletonPs.cpp \
    spectrogram.cpp \
    spectrum.cpp \
    transferprogressdialog.cpp \
    uartserial.cpp \
    udpcon.cpp \
    udppl1gcon.cpp \
    udppl_10gcon.cpp

HEADERS += \
    IUDPConnection.h \
    Proto.h \
    RegDef.h \
    Utils.h \
    connectiontype.h \
    devicesetup.h \
    ethernetsocket.h \
    ethernetsocket10G.h \
    fileprocessing.h \
    fileprocessingconf.h \
    FileTransferAgent.h \
    log.h \
    mainwindow.h \
    matfileprocessing.h \
    qcustomplot.h \
    qspectrogram.h \
    rf.h \
    selftest.h \
    serial_port_singletonPl.h \
    serial_port_singletonPs.h \
    spectrogram.h \
    spectrum.h \
    transferprogressdialog.h \
    uartserial.h \
    udpcon.h \
    udppl1gcon.h \
    udppl_10gcon.h

FORMS += \
    connectiontype.ui \
    devicesetup.ui \
    fileprocessing.ui \
    mainwindow.ui \
    rf.ui \
    selftest.ui \
    spectrum.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    TargetEchoGen.qrc

QTPLUGIN += qled

DISTFILES += \
    dark_mode_style_sheet.qss


# Paths
FFTW_PATH = $$PWD/fftwlib
QWTLIB_PATH = $$PWD/qwtlib\qwt-6.3.0\lib

RELEASE_LIB_PATH = $$PWD/release
DEBUG_LIB_PATH   = $$PWD/debug

# Include and dependency paths
INCLUDEPATH += $$FFTW_PATH
DEPENDPATH  += $$FFTW_PATH

LIBS += -L$$FFTW_PATH -lfftw3
LIBS += -L$$QWTLIB_PATH -lqwt

INCLUDEPATH += $$PWD/qwtlib/qwt-6.3.0/include
DEPENDPATH += $$PWD/qwtlib/qwt-6.3.0/include
