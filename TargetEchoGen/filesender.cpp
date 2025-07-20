#include "filesender.h"
#include "log.h"
#include <QDebug>
void FileSender::run()
{
    // if (TransferMode == SendBulk) {
    //     quint64 totalSize = QFile(filePath).size();
    //     sendFileBulk(startAddress, totalSize); // use real value
    // } else if (TransferMode == ReceiveBulk) {
    //     qint64 bytesRead = 0;
    //     receiveFileBulk(startAddress, expectedSize, &bytesRead);
    // } else {
    //     runStreaming(); // your original chunked sender fallback
    // }
    // em
    // int cnt = 0;
    // while(cnt++ < 1000){
    //     LOG_ONLY_DATA("LOOP %d\n",cnt);
    // }
    receiveFileBulkPS1G(0x40000, NULL);

}
int FileSender::receiveFileBulkPS1G(unsigned int startAddress, qint64* numBytesRdSuccess)
{
    QFile throughputLog("test_10G.log");
    throughputLog.open(QIODevice::Append);
    throughputLog.write(QTime::currentTime().toString("hh:mm:ss:zzz").toUtf8() + '\n');
    throughputLog.write(filePath.toUtf8() + '\n');

    FILE* pFile = fopen(filePath.toUtf8().constData(), "wb");
    if (!pFile || !EthPs01G || !EthPs01G->getConStatus()) {
        LOG_TO_FILE("Error: EthPs01G unavailable or file open failed");
        return -2;
    }

    const int maxReadChunk = MAX_BULK_READ_DATA_SIZE;
    char* rDevBuf = new char[maxReadChunk];
    char ucBuffer[10000] = {0};
    char* byArrPkt = nullptr;
    Proto protocol;

    qint64 numByReadDone = 0;
    qint64 numByToRd = 0;

    while (readSize > 0 && !abort) {
        numByToRd = qMin(readSize, static_cast<qint64>(maxReadChunk));
        LOG_TO_FILE("[FileSender] Requesting bytes: %lld", numByToRd);
        int pktLen = protocol.mPktBulkRead(startAddress, readSize, &byArrPkt);
        EthPs01G->sendMessage(byArrPkt, pktLen, EthPs01G->remote_ip, EthPs01G->remote_port);
        qint64 sizeReceived = 0;
        while (sizeReceived < numByToRd) {
            QHostAddress senderIp(EthPs01G->remote_ip);
            int nDatagram = EthPs01G->readResponsPacket(ucBuffer, sizeof(ucBuffer), senderIp, EthPs01G->remote_port);
            LOG_TO_FILE("[FileSender] nDatagram %d",nDatagram);
            if (nDatagram > 12) {
                protocol.mPktParseBulkRead(ucBuffer);
                LOG_TO_FILE("[FileSender]m_nPacketLength:%d",protocol.m_nPacketLength);
                if (protocol.m_nPacketLength != 0) {
                    fwrite(&ucBuffer[12], 1, protocol.m_nPacketLength-12, pFile);
                    fflush(pFile);
                    sizeReceived += protocol.m_nPacketLength -12;
                    bytesTransferred += protocol.m_nPacketLength - 12;
                    readSize -= (protocol.m_nPacketLength - 12); //12 byte header size
                }
            }
        }
        numByReadDone += sizeReceived;
        startAddress += sizeReceived;
        LOG_TO_FILE("[FileSender] Chunk received: %lld bytes, Remaining: %lld", sizeReceived, readSize);
    }
    //if (numBytesRdSuccess)
    //*numBytesRdSuccess = numByReadDone;
    fclose(pFile);
    delete[] rDevBuf;
    throughputLog.write(QTime::currentTime().toString("hh:mm:ss:zzz").toUtf8() + '\n');
    throughputLog.close();
    LOG_TO_FILE("[FileSender] Upload complete: %lld bytes read", numByReadDone);
    return 0;
}

// void FileSender::setupProgressTimer()
// {
//     progressTimer = new QTimer(this);
//     connect(progressTimer, &QTimer::timeout, this, [this]() {
//         emit progressUpdated(bytesTransferred);
//     });
//     progressTimer->start(500); // Emit signal every 500 ms
// }

// int FileSender::receiveFileBulkPS1G(unsigned int startAddress, qint64* numBytesRdSuccess)
// {
//     QFile throughputLog("test_10G.log");
//     throughputLog.open(QIODevice::Append);
//     throughputLog.write(QTime::currentTime().toString("hh:mm:ss:zzz").toUtf8() + '\n');
//     throughputLog.write(filePath.toUtf8() + '\n');

//     FILE* pFile = fopen(filePath.toUtf8().constData(), "wb");
//     if (!pFile || !EthPs01G || !EthPs01G->getConStatus()) {
//         LOG_TO_FILE("Error EthPs01G EthPs01G->getConStatus pFile not set proper");
//         return -2;
//     }

//     const int maxReadChunk = 8096;
//     char* rDevBuf = new char[maxReadChunk];
//     char ucBuffer[10000] = {0};
//     char* byArrPkt = nullptr;

//     qint64 numByReadDone = 0;
//     qint64 numByToRd = 0;
//     qint64 sizeReceived = 0;
//     Proto protocol;

//     while (this->readSize > 0 && !abort) {
//         numByToRd = qMin(this->readSize, static_cast<qint64>(maxReadChunk));
//         LOG_TO_FILE("[FileSender] Requesting bytes: %lld", numByToRd);

//         int pktLen = protocol.mPktBulkRead(startAddress, this->readSize, &byArrPkt);
//         EthPs01G->sendMessage(byArrPkt, pktLen, EthPs01G->remote_ip, EthPs01G->remote_port);
//         sizeReceived = 0;
//         while (sizeReceived < numByToRd) {
//             QHostAddress senderIp(EthPs01G->remote_ip);
//             int nDatagram = EthPs01G->readResponsPacket(ucBuffer, sizeof(ucBuffer), senderIp, EthPs01G->remote_port);

//             if (nDatagram > 12) {
//                 protocol.mPktParseBulkRead(ucBuffer);
//                 if (protocol.m_nPacketLength != 0) {
//                     fwrite(&ucBuffer[12], 1, protocol.m_nPacketLength, pFile);
//                     fflush(pFile);
//                     sizeReceived += protocol.m_nPacketLength;
//                     readSize -= sizeReceived;
//                     bytesTransferred += protocol.m_nPacketLength; // For progress signaling
//                 }
//             }
//         }
//         //LOG_TO_FILE("[FileSender] Requested bytes: %d", numByToRd);
//         readSize -= sizeReceived;
//         numByReadDone += sizeReceived;
//         LOG_TO_FILE("[FileSender] Requested bytes: %d", numByToRd);
//         startAddress += sizeReceived;
//     }
//     //*numBytesRdSuccess = numByReadDone;
//     fclose(pFile);
//     delete[] rDevBuf;
//     throughputLog.write(QTime::currentTime().toString("hh:mm:ss:zzz").toUtf8() + '\n');
//     throughputLog.close();
//     LOG_TO_FILE("[FileSender] Upload complete: %lld bytes read", numByReadDone);
//     return 0;
// }


// int FileSender::sendFileBulk(unsigned int startAddress, unsigned int size)
// {
//     const int maxChunkSize = MAX_BYTES_WRITE_AT_ONCE;
//     const int bufferLimit  = 0x10A000;
//     LOG_TO_FILE("[Start] FileSender::sendFileBulk");
//     LOG_TO_FILE("Target IP: %s Port: %d",ipAddress.toStdString().c_str(),port);
//     LOG_TO_FILE("File: %s Size: %d  Start Address: 0X%08X",filePath.toStdString().c_str(),size,startAddress);

//     FILE *pFile = fopen(filePath.toUtf8().constData(), "rb");
//     if (!pFile) {
//         LOG_TO_FILE("Error: Unable to open file for reading");
//         return -2;
//     }

//     bool sendComplete = false;
//     while (!sendComplete && !abort) {
//         char *packetBuffer = new char[bufferLimit];
//         int packetBufferSize = 0;

//         while (packetBufferSize < (bufferLimit - (maxChunkSize + 12))) {
//             char rawBuffer[maxChunkSize] = {0};
//             int readLen = qMin(size, static_cast<unsigned int>(maxChunkSize));
//             int count = fread(rawBuffer, sizeof(char), readLen, pFile);

//             if (count <= 0 || ferror(pFile)) {
//                 LOG_TO_FILE("Error: Failed reading from file.");
//                 fclose(pFile);
//                 delete[] packetBuffer;
//                 return -3;
//             }

//             Proto protocol;
//             char *byArrPkt = nullptr;
//             int pktLen = protocol.mPktBulkWrite(startAddress, rawBuffer, count, &byArrPkt);
//             if (!byArrPkt || pktLen <= 0) {
//                 LOG_TO_FILE("Error: Packet creation failed.");
//                 fclose(pFile);
//                 delete[] packetBuffer;
//                 return -4;
//             }
//             memcpy(packetBuffer + packetBufferSize, byArrPkt, pktLen);
//             packetBufferSize += pktLen;
//             delete[] byArrPkt;

//             size -= count;
//             if (size <= 0) {
//                 sendComplete = true;
//                 break;
//             }
//         }
//         int ret = udpSocket->writeDatagram(packetBuffer, packetBufferSize, QHostAddress(ipAddress), port);
//         delete[] packetBuffer;

//         if (ret < 0) {
//             LOG_TO_FILE("Error: Datagram transmission failed. Return");
//             fclose(pFile);
//             return ret * 1000;
//         }

//         LOG_TO_FILE("Data chunk transmitted: %d bytes",packetBufferSize);
//     }
//     fclose(pFile);
//     LOG_TO_FILE("[End] File transmission complete.");
//     return 0;
// }
