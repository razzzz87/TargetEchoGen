#ifndef LOG_H
#define LOG_H

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>
#include <QByteArray>
#include <QWidget>
#include <QString>
#include <cstdarg>

//Keep these value 0 or for enable disable
#define ENABLE_DEBUG    1
#define ENABLE_INFO     1
#define ENABLE_ERROR    1
#define ENABLE_WARNING  1

#define LogFileName "logfile.log"

// 🔹 Log level macros
#define LOG_INFO(format, ...)    Log::logInfo(format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...)   Log::logError(format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...) Log::logWarning(format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...)   Log::logDebug(format, ##__VA_ARGS__)

// 🔹 Legacy macros
#define LOG_TO_FILE(format, ...)       Log::logToFile(format, ##__VA_ARGS__)
#define LOG_ONLY_DATA(format, ...)     Log::logToFileOnlyData(format, ##__VA_ARGS__)

class Log
{
public:
    Log();

    // 🔹 General logging
    static void logToFile(const char* format, ...);
    static void logToFileOnlyData(const char* format, ...);

    // 🔹 Level-specific logging
    static void logInfo(const char* format, ...);
    static void logError(const char* format, ...);
    static void logWarning(const char* format, ...);
    static void logDebug(const char* format, ...);

    // 🔹 Diagnostic utilities
    static void printHexCStyle(const QByteArray& buffer);
    static void printHexRecvBuffer(char* buffer, int len);
    static void showStatusMessage(QWidget* parent, const QString& logText, const QString& dialogText);

private:
    static void logWithLevel(const char* levelTag, const char* format, va_list args);
    static QMutex mutex;
};

namespace BitUtils {

inline uint32_t setBit(uint32_t& value, int pos) {

    LOG_INFO("[BitUtils::setBit] Before: Val:0x08X, Pos:%d",value,pos);
    value |= (1U << pos);
    LOG_INFO("[BitUtils::setBit] After: 0x08X,",value);
    return value;
}

inline uint64_t setBit64(uint64_t& value, int pos) {

    LOG_INFO("[BitUtils::setBit] Before: Val:0x08X, Pos:%d",value,pos);
    value |= (1U << pos);
    LOG_INFO("[BitUtils::setBit] After: 0x08X,",value);
    return value;
}

inline uint32_t clearBit(uint32_t& value, int pos) {

    LOG_INFO("[BitUtils::clearBit] Before: Val:0x08X, Pos:%d",value,pos);
    value &= ~(1U << pos);
    LOG_INFO("[BitUtils::clearBit] After: Val:0x08X, Pos:%d",value,pos);
    return value;
}
inline uint64_t clearBit64(uint64_t& value, int pos) {

    LOG_INFO("[BitUtils::clearBit] Before: Val:0x08X, Pos:%d",value,pos);
    value &= ~(1U << pos);
    LOG_INFO("[BitUtils::clearBit] After: Val:0x08X, Pos:%d",value,pos);
    return value;
}

inline uint32_t setBits(uint32_t value, int start, int end) {
    uint32_t mask = ((1U << (end - start + 1)) - 1) << start;
    return value | mask;
}
inline uint32_t clearBits(uint32_t value, int start, int end) {
    uint32_t mask = ~(((1U << (end - start + 1)) - 1) << start);
    return value & mask;
}
inline uint32_t setValueInBits19to12(uint32_t reg, uint8_t value) {
    const uint32_t mask = 0xFF << 12;          // Bits 19:12
    return (reg & ~mask) | ((value & 0xFF) << 12);
}

inline uint16_t extractBits15to0(uint32_t value) {
    return static_cast<uint16_t>(value & 0xFFFF);
}

// Add more as needed...
}
#endif // LOG_H
