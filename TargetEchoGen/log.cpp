#include "log.h"

Log::Log() {}

void Log::logToFile(const char *sourceFile, const char *functionName, const char *format, ...) {
    QFile file(LogFileName);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);

    // Get the current time
    QString datetime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // Write the timestamp, source file, and function name to the file
    out <<""<<datetime << ": " << sourceFile << " " << functionName << "-";

    // Handle the variable arguments
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    out << buffer;
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
void Log::logToFileOnlyData(const char *format, ...) {
    QFile file(LogFileName);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);
    // Handle the variable arguments
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    out << buffer;
}

void Log::printHexCStyle(const QByteArray &buffer)
{
    LOG_TO_FILE("Received Data Start:");
    for (char byte : buffer) {
        LOG_ONLY_DATA("%02X ", static_cast<unsigned char>(byte));
    }
    LOG_ONLY_DATA("\n");
    LOG_TO_FILE("Received Data End");
}

void Log::printHexRecvBuffer(char *buffer,int len)
{
    LOG_TO_FILE("Received Data Start:");
    for (int i = 0 ; i < len; i++) {
        LOG_ONLY_DATA("%02X ", (unsigned char)buffer[i]);
    }
    LOG_ONLY_DATA("\n");
    LOG_TO_FILE("Received Data End:");
}
