#ifndef LOG_H
#define LOG_H
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <cstdarg>
#include <cstdio>
#include <QMutex>
#include <QMutexLocker>

#define LogFileName "logfile.log"
#define LOG_TO_FILE(format, ...) Log::logToFile(__FILE__, __FUNCTION__, (std::string(format) + "\n").c_str(), ##__VA_ARGS__)
#define LOG_ONLY_DATA(format, ...) Log::logToFileOnlyData(format, ##__VA_ARGS__)

class Log
{
public:
    Log();
    static void logToFile(const char *sourceFile, const char *functionName, const char *format, ...);
    static void logToFileOnlyData(const char *format, ...);
    static void printHexCStyle(const QByteArray &buffer);
    static void printHexRecvBuffer(char* buffer, int len);
private:
    static QMutex mutex;
};
#endif // LOG_H
