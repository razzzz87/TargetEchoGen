QT  += core gui
QT  += network
QT  += serialport
QT  += printsupport
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
win32:LIBS += -lws2_32
QMAKE_CXXFLAGS += -Wa,-mbig-obj

# ------------ NPCAP SDK (LOCAL COPY) ----------------

NPCAP_PATH = $$PWD/3rdparty/npcap
INCLUDEPATH += $$NPCAP_PATH/Include
LIBS += -L$$NPCAP_PATH/Lib/x64 -lwpcap -lPacket -lws2_32

# -----------------------------------------------------

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Utils.cpp \
    connectionctx.cpp \
    connectiontype.cpp \
    dachelper.cpp \
    devicesetup.cpp \
    devicesetuphelper.cpp \
    ethernetsocket.cpp \
    ethernetsocket10G.cpp \
    ethernetsocketpl1g.cpp \
    fileprocessing.cpp \
    FileTransferAgent.cpp \
    fileprocessinghelper.cpp \
    log.cpp \
    main.cpp \
    mainwindow.cpp \
    matfileprocessing.cpp \
    packetforwarder.cpp \
    playbackhelper.cpp \
    proto.cpp \
    qcustomplot.cpp \
    qspectrogram.cpp \
    rf.cpp \
    selftest.cpp \
    spectrogram.cpp \
    spectrum.cpp \
    transferprogressdialog.cpp \
    uartserial.cpp \
    udpcon.cpp

HEADERS += \
    AvrRegAddrDef.h \
    IUDPConnection.h \
    RegDef.h \
    Utils.h \
    connectionctx.h \
    connectiontype.h \
    dachelper.h \
    devicesetup.h \
    devicesetuphelper.h \
    ethernetsocket.h \
    ethernetsocket10G.h \
    ethernetsocketpl1g.h \
    fileprocessing.h \
    FileTransferAgent.h \
    fileprocessinghelper.h \
    log.h \
    mainwindow.h \
    matfileprocessing.h \
    packetforwarder.h \
    playbackhelper.h \
    proto.h \
    qcustomplot.h \
    qspectrogram.h \
    rf.h \
    selftest.h \
    spectrogram.h \
    spectrum.h \
    transferprogressdialog.h \
    uartserial.h \
    udpcon.h

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
    CMakeLists.txt \
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

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/3rdparty/npcap/Lib/release/ -lPacket
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/3rdparty/npcap/Lib/debug/ -lPacket
else:unix:!macx: LIBS += -L$$PWD/3rdparty/npcap/Lib/ -lPacket

INCLUDEPATH += $$PWD/3rdparty/npcap/Include
DEPENDPATH += $$PWD/3rdparty/npcap/Include
