#include "Log.h"
#include "log.h"
#include <QMessageBox>

QMutex Log::mutex;
Log::Log() {}

void Log::logToFile(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logWithLevel("[LOG]", format, args);
    va_end(args);
}

void Log::logToFileOnlyData(const char* format, ...) {
    QMutexLocker locker(&mutex);
    QFile file(LogFileName);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        char buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        out << buffer << "\n";
        file.close();
    }
}

void Log::logInfo(const char* format, ...) {

    if (!ENABLE_INFO) return;

    va_list args;
    va_start(args, format);
    logWithLevel("[INFO]", format, args);
    va_end(args);
}

void Log::logError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logWithLevel("[ERROR]", format, args);
    va_end(args);
}

void Log::logWarning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logWithLevel("[WARNING]", format, args);
    va_end(args);
}

void Log::logDebug(const char* format, ...) {

    if (!ENABLE_DEBUG) return;
    va_list args;
    va_start(args, format);
    logWithLevel("[DEBUG]", format, args);
    va_end(args);
}

void Log::logWithLevel(const char* levelTag, const char* format, va_list args) {
    QMutexLocker locker(&mutex);
    QFile file(LogFileName);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), format, args);
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ")
            << levelTag << " " << buffer << "\n";
        file.close();
    }
}

void Log::printHexCStyle(const QByteArray& buffer) {
    QString hex;
    for (int i = 0; i < buffer.size(); ++i) {
        hex += QString("0x%1, ").arg((unsigned char)buffer[i], 2, 16, QLatin1Char('0')).toUpper();
    }
    LOG_DEBUG("Hex Dump: %s", qPrintable(hex));
}

void Log::printHexRecvBuffer(char* buffer, int len) {
    QString hex;
    for (int i = 0; i < len; ++i) {
        hex += QString("0x%1, ").arg((unsigned char)buffer[i], 2, 16, QLatin1Char('0')).toUpper();
    }
    LOG_DEBUG("PKT:%s", qPrintable(hex));
}

void Log::showStatusMessage(QWidget* parent, const QString& logText, const QString& dialogText) {
    LOG_TO_FILE("Status Alert: %s", logText.toUtf8().constData());
    QMessageBox msgBox(parent);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(dialogText);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();

    LOG_TO_FILE("Closed status dialog: %s", dialogText.toUtf8().constData());
}
