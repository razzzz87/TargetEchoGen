#include "UartSerial.h"
#include "log.h"        // Must provide LOG_INFO and LOG_ERROR macros

#ifdef _WIN32
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
#define SERIAL_EV_RXCHAR 0x0001
#endif
#ifndef SERIAL_EV_BREAK
#define SERIAL_EV_BREAK 0x0008
#endif
#ifndef SERIAL_EV_ERR
#define SERIAL_EV_ERR 0x0080
#endif

UartSerial* UartSerial::instance = nullptr;

UartSerial::UartSerial()
#ifdef _WIN32
    : handle(INVALID_HANDLE_VALUE), isSerial(false)
#else
    : fd(-1), isSerial(false)
#endif
{
    LOG_INFO("[UartSerial] Constructor");
}

UartSerial::~UartSerial() {
    LOG_INFO("[UartSerial] Destructor");
    closePort();
}

#ifdef _WIN32
bool UartSerial::configureUartWithDCB(HANDLE hSerial, int baudRate)
{
    LOG_INFO("[configureUartWithDCB] Enter | Baud: %d", baudRate);
    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(hSerial, &dcb)) {
        LOG_ERROR("[configureUartWithDCB] GetCommState failed. Error: %lu", GetLastError());
        return false;
    }

    dcb.BaudRate = static_cast<DWORD>(baudRate);
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
        LOG_ERROR("[configureUartWithDCB] SetCommState failed. Error: %lu", GetLastError());
        return false;
    }

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 100;
    timeouts.ReadTotalTimeoutConstant = 100;
    timeouts.ReadTotalTimeoutMultiplier = 20;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        LOG_ERROR("[configureUartWithDCB] SetCommTimeouts failed. Error: %lu", GetLastError());
        return false;
    }

    DWORD eventMask = EV_RXCHAR | EV_TXEMPTY | EV_BREAK | EV_ERR;
    if (!SetCommMask(hSerial, eventMask)) {
        LOG_ERROR("[configureUartWithDCB] SetCommMask failed. Error: %lu", GetLastError());
        return false;
    }

    LOG_INFO("[configureUartWithDCB] Config applied: Baud=%lu ByteSize=%u Parity=%u StopBits=%u",
             dcb.BaudRate, dcb.ByteSize, dcb.Parity, dcb.StopBits);
    return true;
}
#else
bool UartSerial::configureUartLinux(int baudRate)
{
    LOG_INFO("[configureUartLinux] Enter | Baud: %d", baudRate);
    termios tty{};
    if (tcgetattr(fd, &tty) != 0) {
        LOG_ERROR("[configureUartLinux] tcgetattr failed: %s", strerror(errno));
        return false;
    }

    // Note: caller should pass platform-specific B* constant or helper to map integer baud
    if (cfsetospeed(&tty, static_cast<speed_t>(baudRate)) != 0 ||
        cfsetispeed(&tty, static_cast<speed_t>(baudRate)) != 0) {
        // Many systems expect B9600 etc. If mapping required, implement mapping helper.
        LOG_ERROR("[configureUartLinux] cfsetospeed/cfsetispeed failed or unsupported baud value");
        // Continue attempting to set attributes anyway
    }

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN]  = 1;
    tty.c_cc[VTIME] = 1;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        LOG_ERROR("[configureUartLinux] tcsetattr failed: %s", strerror(errno));
        return false;
    }

    LOG_INFO("[configureUartLinux] Config applied.");
    return true;
}
#endif

bool UartSerial::openPort(const std::string& portName, int baudRate)
{
    LOG_INFO("[openPort] Enter | Port: %s Baud: %d", portName.c_str(), baudRate);

#ifdef _WIN32
    handle = CreateFileA(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        LOG_ERROR("[openPort] CreateFileA failed for %s | Error: %lu", portName.c_str(), GetLastError());
        return false;
    }

    if (!SetupComm(handle, 4096, 4096)) {
        LOG_ERROR("[openPort] SetupComm failed. Error: %lu", GetLastError());
        CloseHandle(handle);
        return false;
    }

    if (!PurgeComm(handle, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR)) {
        LOG_ERROR("[openPort] PurgeComm failed. Error: %lu", GetLastError());
        CloseHandle(handle);
        return false;
    }

    if (!configureUartWithDCB(handle, baudRate)) {
        LOG_ERROR("[openPort] configureUartWithDCB failed");
        CloseHandle(handle);
        return false;
    }

    LOG_INFO("[openPort] Port opened (Windows): %s", portName.c_str());
#else
    fd = ::open(portName.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        LOG_ERROR("[openPort] open() failed for %s | errno=%d (%s)", portName.c_str(), errno, strerror(errno));
        return false;
    }

    if (!configureUartLinux(baudRate)) {
        LOG_ERROR("[openPort] configureUartLinux failed for %s", portName.c_str());
        ::close(fd);
        fd = -1;
        return false;
    }

    LOG_INFO("[openPort] Port opened (Linux): %s", portName.c_str());
#endif

    isSerial = true;
    LOG_INFO("[openPort] Exit | Port ready");
    return true;
}

void UartSerial::closePort()
{
    LOG_INFO("[closePort] Enter");
#ifdef _WIN32
    if (handle != INVALID_HANDLE_VALUE) {
        if (!PurgeComm(handle, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR)) {
            LOG_ERROR("[closePort] PurgeComm failed. Error: %lu", GetLastError());
        }
        CloseHandle(handle);
        handle = INVALID_HANDLE_VALUE;
        LOG_INFO("[closePort] Handle closed (Windows)");
    } else {
        LOG_INFO("[closePort] Handle already invalid");
    }
#else
    if (fd >= 0) {
        if (::close(fd) != 0) {
            LOG_ERROR("[closePort] close() failed | errno=%d (%s)", errno, strerror(errno));
        } else {
            LOG_INFO("[closePort] fd closed (Linux)");
        }
        fd = -1;
    } else {
        LOG_INFO("[closePort] fd already closed");
    }
#endif
    isSerial = false;
    LOG_INFO("[closePort] Exit");
}

bool UartSerial::sendData(const char* data, int len)
{
    LOG_INFO("[sendData] Enter | Len: %d", len);
    if (!isSerial) {
        LOG_ERROR("[sendData] Serial port not open");
        return false;
    }
    if (!data || len <= 0) {
        LOG_ERROR("[sendData] Invalid buffer or length");
        return false;
    }

#ifdef _WIN32
    DWORD written = 0;
    if (!WriteFile(handle, data, static_cast<DWORD>(len), &written, nullptr)) {
        DWORD err = GetLastError();
        LOG_ERROR("[sendData] WriteFile failed | Error: %lu", err);
        return false;
    }
    if (written != static_cast<DWORD>(len)) {
        LOG_ERROR("[sendData] Partial write | Expected: %d Written: %lu", len, written);
        return false;
    }
#else
    ssize_t sent = ::write(fd, data, static_cast<size_t>(len));
    if (sent < 0) {
        LOG_ERROR("[sendData] write() failed | errno=%d (%s)", errno, strerror(errno));
        return false;
    }
    if (sent != static_cast<ssize_t>(len)) {
        LOG_ERROR("[sendData] Partial write | Expected: %d Written: %zd", len, sent);
        return false;
    }
#endif

    LOG_INFO("[sendData] Exit | Sent %d bytes successfully", len);
    return true;
}

bool UartSerial::receiveData(std::vector<uint8_t>& buffer, int maxLen)
{
    LOG_INFO("[receiveData(vector)] Enter | MaxLen: %d", maxLen);
    if (!isSerial) {
        LOG_ERROR("[receiveData(vector)] Serial port not open");
        return false;
    }
    if (maxLen <= 0) {
        LOG_ERROR("[receiveData(vector)] Invalid maxLen: %d", maxLen);
        return false;
    }

    buffer.resize(static_cast<size_t>(maxLen));

#ifdef _WIN32
    DWORD bytesRead = 0;
    if (!ReadFile(handle, buffer.data(), static_cast<DWORD>(maxLen), &bytesRead, nullptr)) {
        LOG_ERROR("[receiveData(vector)] ReadFile failed | Error: %lu", GetLastError());
        return false;
    }
    buffer.resize(static_cast<size_t>(bytesRead));
    LOG_INFO("[receiveData(vector)] Exit | Received %lu bytes", bytesRead);
    return bytesRead > 0;
#else
    ssize_t received = ::read(fd, buffer.data(), static_cast<size_t>(maxLen));
    if (received < 0) {
        LOG_ERROR("[receiveData(vector)] read() failed | errno=%d (%s)", errno, strerror(errno));
        return false;
    }
    if (received == 0) {
        LOG_ERROR("[receiveData(vector)] read() returned 0 (no data)");
        return false;
    }
    buffer.resize(static_cast<size_t>(received));
    LOG_INFO("[receiveData(vector)] Exit | Received %zd bytes", received);
    return true;
#endif
}

bool UartSerial::receiveData(char* buffer, int maxLen)
{
    LOG_INFO("[receiveData(char*)] Enter | MaxLen: %d", maxLen);
    if (!isSerial) {
        LOG_ERROR("[receiveData(char*)] Serial port not open");
        return false;
    }
    if (!buffer || maxLen <= 0) {
        LOG_ERROR("[receiveData(char*)] Invalid arguments");
        return false;
    }

#ifdef _WIN32
    DWORD bytesRead = 0;
    if (!ReadFile(handle, buffer, static_cast<DWORD>(maxLen), &bytesRead, nullptr)) {
        LOG_ERROR("[receiveData(char*)] ReadFile failed | Error: %lu", GetLastError());
        return false;
    }
    if (bytesRead == 0) {
        LOG_ERROR("[receiveData(char*)] No data read");
        return false;
    }
    LOG_INFO("[receiveData(char*)] Exit | Received %lu bytes", bytesRead);
    return true;
#else
    ssize_t received = ::read(fd, buffer, static_cast<size_t>(maxLen));
    if (received < 0) {
        LOG_ERROR("[receiveData(char*)] read() failed | errno=%d (%s)", errno, strerror(errno));
        return false;
    }
    if (received == 0) {
        LOG_ERROR("[receiveData(char*)] read() returned 0 (no data)");
        return false;
    }
    LOG_INFO("[receiveData(char*)] Exit | Received %zd bytes", received);
    return true;
#endif
}

bool UartSerial::isOpen() const {
    return isSerial;
}

UartSerial* UartSerial::getInstance() {
    LOG_INFO("[getInstance] Enter");
    if (!instance) {
        LOG_INFO("[getInstance] instance is null");
    } else {
        LOG_INFO("[getInstance] instance exists");
    }
    return instance;
}

UartSerial* UartSerial::create(const std::string& portName, int baudRate)
{
    LOG_INFO("[create] Enter | Port: %s Baud: %d", portName.c_str(), baudRate);
    if (instance == nullptr) {
        LOG_INFO("[create] Allocating instance");
        instance = new UartSerial();
        if (!instance->openPort(portName, baudRate)) {
            LOG_ERROR("[create] openPort failed for %s", portName.c_str());
            delete instance;
            instance = nullptr;
            LOG_INFO("[create] Cleanup done");
            return nullptr;
        }
        LOG_INFO("[create] Port opened and instance created");
    } else {
        LOG_INFO("[create] Returning existing instance");
    }
    return instance;
}
