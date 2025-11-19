#include "Log.h"
#include "log.h"
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

QMutex Log::mutex;

// 10 MB
static const qint64 MAX_LOG_SIZE_BYTES = 10 * 1024 * 1024;

Log::Log() {}

// ---- Internal helpers ---------------------------------------------------

void Log::rolloverIfNeeded()
{
    QFileInfo info(LogFileName);
    if (!info.exists())
        return;

    if (info.size() <= MAX_LOG_SIZE_BYTES)
        return;

    QDir dir = info.dir();
    QString baseName = info.completeBaseName();
    QString suffix   = info.completeSuffix();

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString rotatedName = QString("%1_%2").arg(baseName, timestamp);
    if (!suffix.isEmpty())
        rotatedName += "." + suffix;

    QString srcPath = info.absoluteFilePath();
    QString dstPath = dir.absoluteFilePath(rotatedName);

    // Best-effort rename; if it fails we just keep appending to current file
    QFile::rename(srcPath, dstPath);
}

void Log::cleanupOldLogs()
{
    QFileInfo currentInfo(LogFileName);
    QDir dir = currentInfo.dir();
    QString baseName = currentInfo.completeBaseName();

    // Match current and rotated logs: baseName*
    QFileInfoList files = dir.entryInfoList(
        QStringList() << QString("%1*").arg(baseName),
        QDir::Files | QDir::NoSymLinks,
        QDir::Time
        );

    QDateTime cutoff = QDateTime::currentDateTime().addMonths(-1);

    for (const QFileInfo &fi : files) {
        // Never delete the active log file itself
        if (fi.absoluteFilePath() == currentInfo.absoluteFilePath())
            continue;

        if (fi.lastModified() < cutoff) {
            QFile::remove(fi.absoluteFilePath());
        }
    }
}

// ---- Public logging functions -------------------------------------------

void Log::logToFile(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logWithLevel("[LOG]", format, args);
    va_end(args);
}

void Log::logToFileOnlyData(const char* format, ...) {
    QMutexLocker locker(&mutex);

    // Rotate + cleanup BEFORE opening
    rolloverIfNeeded();
    cleanupOldLogs();

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

    // Rotate + cleanup BEFORE opening
    rolloverIfNeeded();
    cleanupOldLogs();

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
