#include "uartserial.h"

#include "UartSerial.h"
#include "log.h"
#ifdef _WIN32
#include <windows.h>
#include <winioctl.h>
#include <windows.h>
#include <winioctl.h>
#include <winbase.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <cstring>
#endif
#ifndef SERIAL_EV_TXEMPTY
#define SERIAL_EV_TXEMPTY 0x0004
#endif
#ifndef SERIAL_EV_RXCHAR
#define SERIAL_EV_RXCHAR   0x0001
#endif

#ifndef SERIAL_EV_TXEMPTY
#define SERIAL_EV_TXEMPTY  0x0004
#endif

#ifndef SERIAL_EV_BREAK
#define SERIAL_EV_BREAK    0x0008
#endif

#ifndef SERIAL_EV_ERR
#define SERIAL_EV_ERR      0x0080
#endif
UartSerial* UartSerial::instance = nullptr;  // Definition
UartSerial::UartSerial() : handle(INVALID_HANDLE_VALUE)
#ifdef _WIN32
    , isSerial(false)
#else
    , fd(-1)
#endif
{
    instance = nullptr;
}

UartSerial::~UartSerial() {
    closePort();
}

bool UartSerial::configureUartWithDCB(HANDLE hSerial, int baudRate)
{
    DCB dcb;
    memset(&dcb, 0, sizeof(DCB));
    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(hSerial, &dcb)) {
        LOG_TO_FILE("GetCommState failed. Error: %lu", GetLastError());
        return false;
    }

    // Apply desired settings
    dcb.BaudRate = baudRate;
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fBinary  = TRUE;
    dcb.fParity  = FALSE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDsrSensitivity = FALSE;
    dcb.fTXContinueOnXoff = TRUE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    dcb.XonChar = 0x11;
    dcb.XoffChar = 0x13;
    dcb.ErrorChar = '?';
    dcb.EofChar = 0;
    dcb.EvtChar = '\n';

    if (!SetCommState(hSerial, &dcb)) {
        LOG_TO_FILE("SetCommState failed. Error: %lu", GetLastError());
        return false;
    }

    COMMTIMEOUTS timeouts;
    memset(&timeouts, 0, sizeof(timeouts));
    timeouts.ReadIntervalTimeout = 100;
    timeouts.ReadTotalTimeoutConstant = 100;
    timeouts.ReadTotalTimeoutMultiplier = 20;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        LOG_TO_FILE("SetCommTimeouts failed. Error: %lu", GetLastError());
        return false;
    }

    DWORD eventMask = EV_RXCHAR | EV_TXEMPTY | EV_BREAK | EV_ERR;
    if (!SetCommMask(hSerial, eventMask)) {
        LOG_TO_FILE("SetCommMask failed. Error: %lu", GetLastError());
        return false;
    }

    // ✅ Final snapshot for diagnostics
    LOG_TO_FILE("UART configuration applied:");
    LOG_TO_FILE("  BaudRate: %lu", dcb.BaudRate);
    LOG_TO_FILE("  ByteSize: %u", dcb.ByteSize);
    LOG_TO_FILE("  Parity: %u", dcb.Parity);
    LOG_TO_FILE("  StopBits: %u", dcb.StopBits);
    LOG_TO_FILE("  DTR: %u, RTS: %u", dcb.fDtrControl, dcb.fRtsControl);
    LOG_TO_FILE("  FlowControl: CTS=%u, DSR=%u, XON/XOFF=%u/%u", dcb.fOutxCtsFlow, dcb.fOutxDsrFlow, dcb.fOutX, dcb.fInX);
    LOG_TO_FILE("  SpecialChars: XON=0x%02X, XOFF=0x%02X, ERR='%c', EOF=0x%02X, EVT='%c'",
                dcb.XonChar, dcb.XoffChar, dcb.ErrorChar, dcb.EofChar, dcb.EvtChar);

    LOG_TO_FILE("Timeouts:");
    LOG_TO_FILE("  ReadInterval: %lu", timeouts.ReadIntervalTimeout);
    LOG_TO_FILE("  ReadTotalConstant: %lu", timeouts.ReadTotalTimeoutConstant);
    LOG_TO_FILE("  ReadTotalMultiplier: %lu", timeouts.ReadTotalTimeoutMultiplier);
    LOG_TO_FILE("  WriteTotalConstant: %lu", timeouts.WriteTotalTimeoutConstant);
    LOG_TO_FILE("  WriteTotalMultiplier: %lu", timeouts.WriteTotalTimeoutMultiplier);

    LOG_TO_FILE("EventMask: 0x%08lX", eventMask);

    return true;
}

bool UartSerial::openPort(const std::string& portName, int baudRate) {
    LOG_TO_FILE("UartSerial::openPort() <ENTER>");
#ifdef _WIN32

    handle = CreateFileA(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        LOG_TO_FILE("Failed to open serial port:  %s [CreateFileA]",portName.c_str());
        return false;
    }
      if (!SetupComm(handle, 4096, 4096)) {
        LOG_TO_FILE("SetupComm failed for port: %s. Error: %lu", portName.c_str(), GetLastError());
        CloseHandle(handle);
        return false;
    }
    LOG_TO_FILE("SetupComm succeeded: InQueue=4096, OutQueue=4096");
    if (!PurgeComm(handle, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR)) {
        LOG_TO_FILE("PurgeComm failed for port: %s. Error: %lu", portName.c_str(), GetLastError());
        CloseHandle(handle);
        return false;
    }
    LOG_TO_FILE("PurgeComm succeeded: RX/TX buffers cleared");
    if(configureUartWithDCB(handle,baudRate)){
        LOG_TO_FILE("Configuration UART configuration\n");
    }
    isSerial = true;
    LOG_TO_FILE("Serial port opened successfully: %s",portName.c_str());
    return true;

#else
    fd = ::open(portName.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        LOG_TO_FILE("Failed to open serial port: " + portName + " [open()]");
        return false;
    }

    termios tty{};
    if (tcgetattr(fd, &tty) != 0) {
        LOG_TO_FILE("Failed to get terminal attributes for port: " + portName + " [tcgetattr]");
        ::close(fd);
        return false;
    }

    cfsetospeed(&tty, baudRate);
    cfsetispeed(&tty, baudRate);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN]  = 1;
    tty.c_cc[VTIME] = 1;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        LOG_TO_FILE("Failed to set terminal attributes for port: " + portName + " [tcsetattr]");
        ::close(fd);
        return false;
    }

    open = true;
    LOG_TO_FILE("Serial port opened successfully: " + portName);
    return true;
#endif
}

void UartSerial::closePort() {
    LOG_TO_FILE("UartSerial::closePort() <ENTER>");
#ifdef _WIN32
    if (handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
        handle = INVALID_HANDLE_VALUE;
    }
#else
    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }
#endif
    isSerial = false;
    LOG_TO_FILE("UartSerial::closePort() <EXIT>");
}

bool UartSerial::sendData(const char* data, int len)
{
    LOG_TO_FILE("UartSerial::sendData() <ENTER>");
    if (!isSerial) {
        LOG_TO_FILE("Error: Serial port not open.");
        return false;
    }
    LOG_TO_FILE("UartSerial::sendData() <ENTER>1 ");
    if (!data || len <= 0) {
        LOG_TO_FILE("Error: Invalid data pointer or length.");
        return false;
    }

    bool success = false;

#ifdef _WIN32
    DWORD written = 0;
    LOG_TO_FILE("UartSerial::sendData() <ENTER>1 ");
    if (!WriteFile(handle, data, len, &written, nullptr)) {
        DWORD errCode = GetLastError();
        LOG_TO_FILE("WriteFile failed. Error code: %lu", errCode);
    } else if (written != static_cast<DWORD>(len)) {
        LOG_TO_FILE("Partial write: Expected %d bytes, wrote %lu bytes", len, written);
    } else {

        success = true;
    }
    LOG_TO_FILE("UartSerial::sendData() <ENTER>2 ");
#else
    ssize_t sent = write(fd, data, len);
    if (sent < 0) {
        LOG_TO_FILE("write() failed: %s", strerror(errno));
    } else if (sent != static_cast<ssize_t>(len)) {
        LOG_TO_FILE("Partial write: Expected %d bytes, wrote %zd bytes", len, sent);
    } else {
        success = true;
    }
#endif

    LOG_TO_FILE("UartSerial::sendData() <EXIT> Status: %s", success ? "Success" : "Failure");
    return success;
}

bool UartSerial::receiveData(std::vector<uint8_t>& buffer, int maxLen) {
    if (!isSerial || maxLen <= 0) return false;
    buffer.resize(maxLen);
#ifdef _WIN32
    DWORD read;
    if (!ReadFile(handle, buffer.data(), maxLen, &read, nullptr)) return false;
    buffer.resize(read);
    return read > 0;
#else
    ssize_t received = read(fd, buffer.data(), maxLen);
    if (received <= 0) return false;
    buffer.resize(received);
    return true;
#endif
}

bool UartSerial::receiveData(char* buffer, int maxLen)
{
    LOG_TO_FILE("UartSerial::receiveData() <ENTER>");

    if (!isSerial || buffer == nullptr || maxLen <= 0) {
        LOG_TO_FILE("UART receive failed: invalid state or arguments.");
        return false;
    }

#ifdef _WIN32
    if (handle == INVALID_HANDLE_VALUE) {
        LOG_TO_FILE("UART receive failed: invalid handle.");
        return false;
    }

    OVERLAPPED ov = {};
    ov.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (!ov.hEvent) {
        LOG_TO_FILE("Failed to create OVERLAPPED event.");
        return false;
    }

    DWORD bytesRead = 0;
    DWORD dwEvent;
    BOOL result=false;
    WaitCommEvent(handle, &dwEvent, nullptr);
    if (dwEvent & EV_RXCHAR) {
        result = ReadFile(handle, buffer, 16, &bytesRead, nullptr);
    }
    if (!result) {
        DWORD err = GetLastError();
        if (err == ERROR_IO_PENDING) {
            DWORD waitResult = WaitForSingleObject(ov.hEvent, 500); // 500ms timeout
            if (waitResult == WAIT_OBJECT_0) {
                if (!GetOverlappedResult(handle, &ov, &bytesRead, FALSE)) {
                    LOG_TO_FILE("GetOverlappedResult failed | Error: %lu", GetLastError());
                    CloseHandle(ov.hEvent);
                    return false;
                }
            } else {
                LOG_TO_FILE("WaitForSingleObject timeout or failure | Code: %lu", waitResult);
                CancelIo(handle);
                CloseHandle(ov.hEvent);
                return false;
            }
        } else {
            LOG_TO_FILE("ReadFile failed | Error: %lu", err);
            CloseHandle(ov.hEvent);
            return false;
        }
    }

    CloseHandle(ov.hEvent);

    if (bytesRead == 0) {
        LOG_TO_FILE("UART receive failed: no data read.");
        return false;
    }

    LOG_TO_FILE("UART received %lu bytes.", bytesRead);
    return true;

#else
    if (fd < 0) {
        LOG_TO_FILE("UART receive failed: invalid file descriptor.");
        return false;
    }

    ssize_t received = ::read(fd, buffer, maxLen);
    if (received <= 0) {
        LOG_TO_FILE("read() failed or returned zero | errno: %d (%s)", errno, strerror(errno));
        return false;
    }

    LOG_TO_FILE("UART received %zd bytes.", received);
    return true;
#endif
}

bool UartSerial::isOpen() const {
    return isSerial;
}



UartSerial* UartSerial::getInstance() {
    if (instance) {
        LOG_TO_FILE("UartSerial::getInstance() <ENTER>");
    } else {
        LOG_TO_FILE("UartSerial::getInstance() instance null");
    }
    return instance;
}

UartSerial* UartSerial::create(const std::string& portName, int baudRate)
{
    LOG_TO_FILE("UartSerial::create() <ENTER>");
    if (instance == nullptr)
    {
        LOG_TO_FILE("UartSerial::create() <ALLOCATING>");
        instance = new UartSerial();
        if (!instance->openPort(portName, baudRate)) {
            LOG_TO_FILE("UartSerial::create() <FAILED TO OPEN PORT>");
            delete instance;
            instance = nullptr;
            LOG_TO_FILE("UartSerial::create() <CLEANUP DONE>");
            return nullptr;
        }
        LOG_TO_FILE("UartSerial::create() <PORT OPENED SUCCESSFULLY>");
    }
    return instance;
}
