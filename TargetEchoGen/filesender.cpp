#include "filesender.h"
#include "log.h"

void FileSender::run()
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_TO_FILE("Failed to open file: %s\n",filePath.toStdString().c_str());
        return;
    }
    LOG_TO_FILE("FilePath %s\n",filePath.toStdString().c_str());
    while (!file.atEnd() && (abort == false)) {
        QMutexLocker locker(&mutex);
        QByteArray buffer = file.read(readSize); // Read in configurable chunks
        locker.unlock();

        udpSocket->writeDatagram(buffer, QHostAddress(ipAddress), port);
        QThread::msleep(10); // Sleep to avoid flooding the network
    }
    if(abort == true)
    {
        LOG_TO_FILE("Info: Forcally exited\n");
    }
    file.close();
    LOG_TO_FILE("File write completed:%s\n",filePath.toStdString().c_str());
}
void FileSender::setReadSize(int newSize)
{
    QMutexLocker locker(&mutex);
    readSize = newSize;
}
void FileSender::setFileSize(QString filePath)
{
    this->filePath = filePath;
}

void FileSender::abortFileWrite(bool abort)
{
    this->abort = abort;
}
