#ifndef UARTSERIAL_H
#define UARTSERIAL_H

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <windows.h>
#include <winioctl.h>

class UartSerial
{
public:
    static UartSerial* getInstance();
    static UartSerial* create(const std::string& portName, int baudRate);
    bool openPort(const std::string& portName, int baudRate = 9600);
    bool configureUartWithDCB(HANDLE handle,int baudRate);
    void closePort();

    bool sendData(const char* data, int len);
    bool receiveData(std::vector<uint8_t>& buffer, int maxLen);
    bool receiveData(char *buffer, int maxLen);
    bool isOpen() const;
    ~UartSerial();
private:
    UartSerial(); // private constructor

#ifdef _WIN32
    void* handle;
#else
    int fd;
#endif
    bool isSerial;
    static UartSerial* instance;
};


#endif // UARTSERIAL_H
