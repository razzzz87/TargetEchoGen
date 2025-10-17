cmake_minimum_required(VERSION 3.16)
project(TargetEchoGen LANGUAGES CXX)

# Use at least C++11 (project used CONFIG += c++11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable automatic Qt features
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Allow big object for MSVC/MinGW if needed
add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-Wa,-mbig-obj>")

# Find Qt (adjust to Qt5 if you use Qt5)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network SerialPort PrintSupport)

# Source files
set(SOURCES
    Utils.cpp
    connectiontype.cpp
    dachelper.cpp
    devicesetup.cpp
    devicesetuphelper.cpp
    ethernetsocket.cpp
    ethernetsocket10G.cpp
    ethernetsocketpl1g.cpp
    fileprocessing.cpp
    FileTransferAgent.cpp
    fileprocessinghelper.cpp
    log.cpp
    main.cpp
    mainwindow.cpp
    matfileprocessing.cpp
    playbackhelper.cpp
    proto.cpp
    qcustomplot.cpp
    qspectrogram.cpp
    rf.cpp
    selftest.cpp
    spectrogram.cpp
    spectrum.cpp
    transferprogressdialog.cpp
    uartserial.cpp
    udpcon.cpp
)

# Header list (optional but useful for IDEs)
set(HEADERS
    AvrRegAddrDef.h
    IUDPConnection.h
    RegDef.h
    Utils.h
    connectiontype.h
    dachelper.h
    devicesetup.h
    devicesetuphelper.h
    ethernetsocket.h
    ethernetsocket10G.h
    ethernetsocketpl1g.h
    fileprocessing.h
    FileTransferAgent.h
    fileprocessinghelper.h
    log.h
    mainwindow.h
    matfileprocessing.h
    playbackhelper.h
    proto.h
    qcustomplot.h
    qspectrogram.h
    rf.h
    selftest.h
    spectrogram.h
    spectrum.h
    transferprogressdialog.h
    uartserial.h
    udpcon.h
)

# UI files
set(UIS
    connectiontype.ui
    devicesetup.ui
    fileprocessing.ui
    mainwindow.ui
    rf.ui
    selftest.ui
    spectrum.ui
)

# Resources
set(RCCS
    TargetEchoGen.qrc
)

# Create executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${UIS}
    ${RCCS}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# FFTW and QWT paths (from .pro)
# Adjust these paths if they are located elsewhere on your system
set(FFTW_PATH "${CMAKE_CURRENT_SOURCE_DIR}/fftwlib")
set(QWTLIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/qwtlib/qwt-6.3.0")

target_include_directories(${PROJECT_NAME} PRIVATE
    "${FFTW_PATH}"
    "${QWTLIB_PATH}/include"
)

# Link libraries
# Link FFTW and QWT using -L paths and names equivalent to the .pro
target_link_directories(${PROJECT_NAME} PRIVATE
    "${FFTW_PATH}"
    "${QWTLIB_PATH}/lib"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::SerialPort
    Qt6::PrintSupport
    # Link FFTW and QWT by name; change if actual library names differ (e.g., libfftw3.dll.a)
    fftw3
    qwt
)

# On Windows link ws2_32 (as in .pro)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

# If you need to expose QTPLUGIN qled, you may need to add it as a library or handle plugin deployment separately.
# QTPLUGIN += qled in .pro typically requires building/including the plugin; handle that as needed for your environment.

# Installation rules (optional)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

# Useful targets
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# Enable parallel builds by default when using CMake --build . --parallel
message(STATUS "Project ${PROJECT_NAME} configured")
