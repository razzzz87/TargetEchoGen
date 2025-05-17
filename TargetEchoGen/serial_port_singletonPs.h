#ifndef SERIAL_PORT_SINGLETONPS_H
#define SERIAL_PORT_SINGLETONPS_H

#include <QSerialPort>
#include <QMutex>
#include <QMutexLocker>

class SerialPortSingletonPs {
public:
    static SerialPortSingletonPs& instance() {
        static SerialPortSingletonPs instance;
        return instance;
    }

    QSerialPort* getSerialPort() {
        return &serialPort;
    }

private:
    QSerialPort serialPort;
    QMutex mutex;

    // Private constructor to prevent instantiation
    SerialPortSingletonPs() {
        // Initialize the serial port settings here if needed
    }

    // Delete copy constructor and assignment operator
    SerialPortSingletonPs(const SerialPortSingletonPs&) = delete;
    SerialPortSingletonPs& operator=(const SerialPortSingletonPs&) = delete;
};
#endif // SERIAL_PORT_SINGLETONPS_H
