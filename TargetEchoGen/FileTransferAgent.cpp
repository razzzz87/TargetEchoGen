#include "FileTransferAgent.h"
#include "log.h"
#include <QDebug>
#include <QtGlobal>

void FileTransferAgent::run()
{
    if(deviceType == PS1G){

        if(this->dir == HostToTarget){
            sendFileBulkPS01G(0x40000,NULL);
        }
        else{
            receiveFileBulkPS1G(0x40000,NULL);
        }
    }
    else if (deviceType == PL1G){

        if(this->dir == HostToTarget){
        }else{

        }
    }
    else if(deviceType == PL10G){

        if(this->dir == HostToTarget){
        }
        else{

        }
    }
    else{

    }
}

void FileTransferAgent::configure(const QString& ip, quint16 portNum, const QString& path, int chunkSize,TransferDirection dir) {
    ipAddress = ip;
    port = portNum;
    filePath = path;
    readSize = chunkSize;
    this->dir = dir;
}

void FileTransferAgent::setupDevice(UDP_PS1G_Con* ps1g) {
    deviceType = PS1G;
    EthPs01G = ps1g;
    EthPl10G = nullptr;
    EthPl01G = nullptr;
}

void FileTransferAgent::setupDevice(UDP_PL10G_Con* pl10g) {
    deviceType = PL10G;
    EthPl10G = pl10g;
    EthPs01G = nullptr;
    EthPl01G = nullptr;
}

void FileTransferAgent::setupDevice(UDP_PL1G_Con* pl1g) {
    deviceType = PL1G;
    EthPl01G = pl1g;
    EthPs01G = nullptr;
    EthPl10G = nullptr;
}

int FileTransferAgent::receiveFileBulkPS1G(unsigned int startAddress, qint64* numBytesRdSuccess)
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

    int updateProgressbarCount = 0;
    const int maxChunkSize = qMin(MAX_BULK_READ_DATA_SIZE, (65000)); // Protocol-safe chunk
    char* rDevBuf = new char[maxChunkSize];
    char ucBuffer[10000] = {0};
    char* byArrPkt = nullptr;
    Proto protocol;

    qint64 numByReadDone = 0;
    qint64 read_req_size = readSize;

    while (readSize > 0 && !abort) {
        qint64 chunkToRequest = qMin(readSize, static_cast<qint64>(maxChunkSize));
        LOG_TO_FILE("[FileTransferAgent] Requesting bytes: %lld", chunkToRequest);

        int pktLen = protocol.mPktBulkRead(startAddress, static_cast<int>(chunkToRequest), &byArrPkt);
        EthPs01G->sendMessage(byArrPkt, pktLen, EthPs01G->remote_ip, EthPs01G->remote_port);

        qint64 sizeReceived = 0;
        while ((sizeReceived < chunkToRequest) && !abort) {
            QHostAddress senderIp(EthPs01G->remote_ip);
            int nDatagram = EthPs01G->readResponsPacket(ucBuffer, sizeof(ucBuffer), senderIp, EthPs01G->remote_port);
            LOG_TO_FILE("[FileTransferAgent] nDatagram %d", nDatagram);

            if (nDatagram > 12) {
                protocol.mPktParseBulkRead(ucBuffer);
                LOG_TO_FILE("[FileTransferAgent] m_nPacketLength: %d", protocol.m_nPacketLength);

                if (protocol.m_nPacketLength != 0) {
                    fwrite(&ucBuffer[12], 1, protocol.m_nPacketLength - 12, pFile);
                    fflush(pFile);
                    sizeReceived += protocol.m_nPacketLength - 12;
                    bytesTransferred += protocol.m_nPacketLength - 12;
                    readSize -= (protocol.m_nPacketLength - 12); // Header offset
                }
            }

            if ((updateProgressbarCount++ >= AFTER_NUMBER_OF_PKT) && !abort) {
                int percentageComplete = static_cast<int>((bytesTransferred * 100.0) / read_req_size);
                emit progressUpdated(percentageComplete);
                updateProgressbarCount = 0;
            }
        }

        numByReadDone += sizeReceived;
        startAddress += static_cast<unsigned int>(sizeReceived);
        LOG_TO_FILE("[FileTransferAgent] Chunk received: %lld bytes, Remaining: %lld", sizeReceived, readSize);
    }

    if (numBytesRdSuccess)
        *numBytesRdSuccess = numByReadDone;

    fclose(pFile);
    delete[] rDevBuf;
    throughputLog.write(QTime::currentTime().toString("hh:mm:ss:zzz").toUtf8() + '\n');
    throughputLog.close();

    LOG_TO_FILE("[FileTransferAgent] Upload complete: %lld bytes read", numByReadDone);
    LOG_TO_FILE(abort ? "File transmission aborted" : "File transmission complete.");

    emit close_progress_pop();  // ✅ Progress dialog cleanup
    return 0;
}

int FileTransferAgent::GetTotalbyte(){
    return TransferReqSize;
}
int FileTransferAgent::GetTransferBbyte(){
    return bytesTransferred;
}
int FileTransferAgent::sendFileBulkPS01G(unsigned int startAddress, unsigned int size)
{
    constexpr int BUFFERED_PACKETS_SIZE = 0x10A000; // 1MB + 40KB

    if (!QFile::exists(filePath)) {
        LOG_TO_FILE("File not found: %s", filePath.toUtf8().constData());
        return -1;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_TO_FILE("File open failed: %s", file.errorString().toUtf8().constData());
        return -2;
    }

    const unsigned int totalSize = file.size();
    unsigned int remainingSize = file.size();
    TransferReqSize  = file.size();
    int updateProgressbarCount = 0;
    bytesTransferred = 0;
    LOG_TO_FILE("=========================================================================");
    LOG_TO_FILE("FileWrite:: startAddress:%u size:%u Filename:%s", startAddress, totalSize, filePath.toUtf8().constData());
    LOG_TO_FILE("==========================================================================");
    abort = false;
    while ((remainingSize > 0) && !abort) {
        int bufferedSize = 0;

        while ((bufferedSize < (BUFFERED_PACKETS_SIZE - (MAX_BYTES_WRITE_AT_ONCE + 12)) && remainingSize > 0 ) && !abort) {
            int chunkSize = qMin(remainingSize, static_cast<unsigned int>(MAX_BYTES_WRITE_AT_ONCE));
            QByteArray chunk = file.read(chunkSize);

            if (chunk.isEmpty()) {
                if (file.error() != QFile::NoError) {
                    LOG_TO_FILE("Read error: %s", file.errorString().toUtf8().constData());
                    file.close();
                    return -3;
                }
                LOG_TO_FILE("End of file reached.");
                break;
            }
            Proto protocol;
            char* packetData = nullptr;
            int packetLen = protocol.mPktBulkWrite(startAddress, chunk.data(), chunk.size(), &packetData);

            if (!packetData || packetLen <= 0) {
                LOG_TO_FILE("Packet creation failed for chunk at addr: %u", startAddress);
                file.close();
                return -4;
            }

            EthPs01G->sendMessage(packetData, packetLen, EthPs01G->remote_ip, EthPs01G->remote_port);
            delete[] packetData;
            startAddress += chunk.size();
            remainingSize -= chunk.size();
            bufferedSize += chunk.size();
            bytesTransferred += chunk.size();

            if((updateProgressbarCount++ < AFTER_NUMBER_OF_PKT) && !abort){
                int percentageComplete = static_cast<int>((bytesTransferred * 100.0) / totalSize);
                emit progressUpdated(percentageComplete);
                updateProgressbarCount = 0;
            }
        }
    }
    file.close();
    if(!abort)LOG_TO_FILE("File transmission complete.");
    else LOG_TO_FILE("File transmission aborted");
    emit close_progress_pop();
    return 0;
}
