#include "FileTransferAgent.h"
#include "log.h"
#include <QDebug>
#include <QtGlobal>
#include <QFileInfo>

void FileTransferAgent::run()
{
    LOG_INFO("FileTransferAgent::run() <ENTER>");
    LOG_INFO("eInterFace:%d",_eInterFace);
    if(_eInterFace == eETHPL1G){

        if(this->_Dir == eWrite){
            LOG_INFO("FileTransferAgent::run()eWrite ");
            WriteFileBulk01G(0x4200,NULL);
        }
        else if(this->_Dir == eStream){

            LOG_INFO("FileTransferAgent::run() call BulkFileReadStreamEth01G ");
            BulkFileReadStreamEth01G(0x4200,NULL);
        }
        else if(this->_Dir == eRead){

            LOG_INFO("FileTransferAgent::run() eRead ");
            BulkReadFileEth01G(0x4200,NULL);
        }
        else{

        }
    }
    else if (_eInterFace == eETH10G){

        if(_Dir == eWrite){
             LOG_INFO("eETH10G FileTransferAgent::run() eWrite ");
            WriteFileBulk10G(0x4200,NULL);
        }
        else if(_Dir == eStream){
            StreamReadFileBulkEth10G(0x4200,NULL);
        }
        else{
            LOG_INFO("eETH10G FileTransferAgent::run() eRead ");
            ReadFileBulkEth10G(0x4200,NULL);
        }
    }
    else if(_eInterFace == eSERIAL){

        if(_Dir == eWrite){
        }
        else{
            LOG_INFO("FileTransferAgent::run() eRead ");
            ReadFileBulkEth10G(0x4200,NULL);
        }
    }
    else{

    }
    LOG_INFO("FileTransferAgent::run() <EXIT>");
}

void FileTransferAgent::configure(const QString& ip, quint16 portNum, const QString& path, int chunkSize,eXferDir dir) {
    _IPAddress = ip;
    _Port = portNum;
    _sFilePath = path;
    _iDataSize = chunkSize;
    _Dir = dir;
}

void FileTransferAgent::configure(stFileReadWriteConf stReadWriteCng){
    _sFilePath = stReadWriteCng.sFilePath;
    _iDataSize = stReadWriteCng.iFileSize;
    _Dir = stReadWriteCng._Dir;
    _eInterFace = stReadWriteCng.eInterface;

}
void FileTransferAgent::setupDevice(iface deviceType) {

    if(deviceType == eETHPL1G){
        this->eth0 = EthernetSocket::getInstance();
    }
    else if(deviceType == eETH10G){
        this->eth10G = EthernetSocket10G::getInstance();
    }else{

    }
}

int FileTransferAgent::BulkFileReadStreamEth01G(unsigned int startAddress, qint64* numBytesRdSuccess)
{
    LOG_INFO("FileTransferAgent::BulkFileReadStreamEth01G <ENTER> ReadSize:%lld, FilePath:%s", (long long)_iDataSize, qPrintable(_sFilePath));

    ethPL01G = EthernetSocketPL1G::getInstance();
    if (!ethPL01G) {
        LOG_ERROR("Error: Eth01G unavailable");
        return 0;
    }

    FILE* pFile = fopen(_sFilePath.toUtf8().constData(), "wb");
    if (!pFile) {
        LOG_ERROR("Error: File open failed %s", qPrintable(_sFilePath));
        return -2;
    }

    const int dataPayloadSize = 1456;    // adjust to 1456 if required
    const int headerSize = 12;

    char ucBuffer[4096];
    char* byArrPkt = nullptr;
    Proto protocol;

    qint64 numByReadDone = 0;
    abort = false;
    while (_iDataSize > 0 && !abort)
    {
        qint64 chunkToRequest = qMin(_iDataSize, static_cast<qint64>(dataPayloadSize));
        int pktLen = protocol.mPktBulkRead(startAddress, static_cast<int>(chunkToRequest), &byArrPkt);
        if (pktLen <= 0 || !byArrPkt) {
            LOG_ERROR("Failed to build bulk-read packet");
            if (byArrPkt) delete[] byArrPkt;
            break;
        }
        //Log::printHexRecvBuffer(byArrPkt,16);

        if (!ethPL01G->sendData(byArrPkt, pktLen)) {
            LOG_ERROR("sendData failed");
            delete[] byArrPkt;
            break;
        }
        delete[] byArrPkt;
        byArrPkt = nullptr;

        qint64 sizeReceivedForChunk = 0;
        while ((sizeReceivedForChunk < chunkToRequest) && !abort)
        {
            int want = headerSize + static_cast<int>(chunkToRequest); // header + requested payload
            int RecvByte;
            bool ok = ethPL01G->receivePacketWithSync2(ucBuffer, want, RecvByte);
            if (ok)
            {
                protocol.m_nPacketLength = protocol.mPktParseBulkRead(ucBuffer);
                size_t payloadLen = protocol.m_nPacketLength - headerSize;
                if (payloadLen > 0)
                {
                    size_t written = fwrite(&ucBuffer[12], 1,chunkToRequest, pFile);
                    fflush(pFile);
                    if (written != chunkToRequest) {
                        LOG_ERROR("fwrite short: wrote %zu of %d", written, written);
                    }
                    bytesTransferred += written;
                    _iDataSize -= written;
                    sizeReceivedForChunk += written;
                    numByReadDone += written;
                }
            }
        }
    } // outer loop
    if (numBytesRdSuccess)
        *numBytesRdSuccess = numByReadDone;

    if (pFile) {
        fclose(pFile);
        pFile = nullptr;
    }

    LOG_INFO("[FileTransferAgent] Read complete: %lld bytes read", (long long)numByReadDone);
    LOG_INFO(abort ? "File transmission aborted" : "File transmission complete.");
    emit transferComplete(eReadDone);
    LOG_INFO("FileTransferAgent::BulkFileReadStreamEth01G <EXIL>");
    return abort ? -1 : 0;
}


int FileTransferAgent::BulkReadFileEth01G(unsigned int startAddress, qint64* numBytesRdSuccess)
{
    LOG_INFO("FileTransferAgent::BulkReadFileEth01G <ENTER> ReadSize:%lld, FilePath:%s", (long long)_iDataSize, qPrintable(_sFilePath));

    ethPL01G = EthernetSocketPL1G::getInstance();
    if (!ethPL01G) {
        LOG_ERROR("Error: Eth01G unavailable");
        return 0;
    }

    FILE* pFile = fopen(_sFilePath.toUtf8().constData(), "wb");
    if (!pFile) {
        LOG_ERROR("Error: File open failed %s", qPrintable(_sFilePath));
        return -2;
    }

    const int dataPayloadSize = 1456;    // adjust to 1456 if required
    const int headerSize = 12;
    const int packetMax = headerSize + dataPayloadSize;

    char ucBuffer[4096];
    char* byArrPkt = nullptr;
    Proto protocol;

    qint64 numByReadDone = 0;
    qint64 read_req_size = _iDataSize;
    int updateProgressbarCount = 0;
    abort = false;
    while (_iDataSize > 0 && !abort)
    {
        qint64 chunkToRequest = qMin(_iDataSize, static_cast<qint64>(dataPayloadSize));
        int pktLen = protocol.mPktBulkRead(startAddress, static_cast<int>(chunkToRequest), &byArrPkt);
        if (pktLen <= 0 || !byArrPkt) {
            LOG_ERROR("Failed to build bulk-read packet");
            if (byArrPkt) delete[] byArrPkt;
            break;
        }

        if (!ethPL01G->sendData(byArrPkt, pktLen)) {
            LOG_ERROR("sendData failed");
            delete[] byArrPkt;
            break;
        }
        delete[] byArrPkt;
        byArrPkt = nullptr;

        qint64 sizeReceivedForChunk = 0;
        while ((sizeReceivedForChunk < chunkToRequest) && !abort)
        {
            int want = headerSize + static_cast<int>(chunkToRequest); // header + requested payload
            int RecvByte;
            bool ok = ethPL01G->receivePacketWithSync2(ucBuffer, want, RecvByte);
            if (ok)
            {
                protocol.m_nPacketLength = protocol.mPktParseBulkRead(ucBuffer);
                size_t payloadLen = protocol.m_nPacketLength - headerSize;
                if (payloadLen > 0)
                {
                    size_t written = fwrite(&ucBuffer[12], 1,chunkToRequest, pFile);
                    fflush(pFile);
                    if (written != chunkToRequest) {
                        LOG_ERROR("fwrite short: wrote %zu of %d", written, written);
                    }
                    bytesTransferred += written;
                    _iDataSize -= written;
                    sizeReceivedForChunk += written;
                    numByReadDone += written;
                }
            }
        }
        if ((updateProgressbarCount++ >= AFTER_NUMBER_OF_PKT) && !abort) {
            int percentageComplete = read_req_size ? static_cast<int>((bytesTransferred * 100.0) / read_req_size) : 100;
            emit progressUpdated(percentageComplete);
            updateProgressbarCount = 0;
        }
        startAddress += static_cast<unsigned int>(sizeReceivedForChunk);
    } // outer loop
    if (numBytesRdSuccess)
        *numBytesRdSuccess = numByReadDone;

    if (pFile) {
        fclose(pFile);
        pFile = nullptr;
    }

    LOG_INFO("[FileTransferAgent] Read complete: %lld bytes read", (long long)numByReadDone);
    LOG_INFO(abort ? "File transmission aborted" : "File transmission complete.");
    emit transferComplete(eReadDone);
    LOG_INFO("FileTransferAgent::BulkReadFileEth01G <EXIL>");
    return abort ? -1 : 0;
}


int FileTransferAgent::ReadFileBulkEth10G(unsigned int startAddress, qint64* numBytesRdSuccess)
{
    LOG_INFO("FileTransferAgent::ReadFileBulkEth10G <ENTER> ReadSize:%d,FilePath:%s",_iDataSize,_sFilePath.toStdString().c_str());

    QFile throughputLog("test_10G.log");

    eth10G = EthernetSocket10G::getInstance();
    if (!eth10G) {
        LOG_ERROR("Error: Eth10G unavailable");
        return 0;
    }

    throughputLog.open(QIODevice::Append);
    throughputLog.write(QTime::currentTime().toString("hh:mm:ss:zzz").toUtf8() + '\n');
    throughputLog.write(_sFilePath.toUtf8() + '\n');

    FILE* pFile = fopen(_sFilePath.toUtf8().constData(), "wb");
    if (!pFile || !eth10G->getConnStatus()) {
        LOG_ERROR("Error: File open failed or Eth10G disconnected");
        return -2;
    }

    const int dataPayloadSize = 1456;
    const int packetSize = dataPayloadSize + 12;
    char ucBuffer[packetSize] = {0};
    char* byArrPkt = nullptr;
    Proto protocol;

    qint64 numByReadDone = 0;
    qint64 read_req_size = _iDataSize;
    int updateProgressbarCount = 0;
    abort = false;
    while (_iDataSize > 0 && !abort) {
        qint64 chunkToRequest = qMin(_iDataSize, static_cast<qint64>(dataPayloadSize));
        int pktLen = protocol.mPktBulkRead(startAddress, static_cast<int>(chunkToRequest), &byArrPkt);
        //Log::printHexRecvBuffer(byArrPkt,16);
        eth10G->sendData(byArrPkt, pktLen, eth10G->RemoteIP.toStdString(), eth10G->Port);

        qint64 sizeReceived = 0;
        while ((sizeReceived < chunkToRequest) && !abort) {

            std::string senderip;
            uint16_t senderPort;
            sizeReceived = eth10G->receiveData(ucBuffer, packetSize, senderip, senderPort);
            if (sizeReceived >= 12)
            {
                protocol.m_nPacketLength = protocol.mPktParseBulkRead(ucBuffer);
                int payloadLen = protocol.m_nPacketLength - 12;
                if (payloadLen > 0)
                {
                        fwrite(&ucBuffer[12], 1, payloadLen, pFile);
                        fflush(pFile);
                        bytesTransferred += payloadLen;
                        _iDataSize -= payloadLen;
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
        //LOG_INFO("Chunk received: %lld bytes, Remaining: %lld, PacketLength: %d", sizeReceived, _ReadSize, protocol.m_nPacketLength);
    }

    if (numBytesRdSuccess)
        *numBytesRdSuccess = numByReadDone;

    fclose(pFile);
    throughputLog.write(QTime::currentTime().toString("hh:mm:ss:zzz").toUtf8() + '\n');
    throughputLog.close();

    LOG_INFO("[FileTransferAgent] Read complete: %lld bytes read", numByReadDone);
    LOG_INFO(abort ? "File transmission aborted" : "File transmission complete.");
    emit transferComplete(eReadDone);
    LOG_INFO("FileTransferAgent::ReadFileBulkEth10G <EXIT>");
    return 0;
}

int FileTransferAgent::StreamReadFileBulkEth10G(unsigned int startAddress, qint64* numBytesRdSuccess)
{
    LOG_INFO("FileTransferAgent::StreamReadFileBulkEth10G <ENTER> ReadSize:%d,FilePath:%s",_iDataSize,_sFilePath.toStdString().c_str());
    QFile throughputLog("test_10G.log");

    eth10G = EthernetSocket10G::getInstance();
    if (!eth10G) {
        LOG_ERROR("Error: Eth10G unavailable");
        return 0;
    }

    throughputLog.open(QIODevice::Append);
    throughputLog.write(QTime::currentTime().toString("hh:mm:ss:zzz").toUtf8() + '\n');
    throughputLog.write(_sFilePath.toUtf8() + '\n');

    FILE* pFile = fopen(_sFilePath.toUtf8().constData(), "wb");
    if (!pFile) {
        LOG_ERROR("Error: File open failed %s",_sFilePath.toStdString().c_str());
        return -2;
    }

    const int dataPayloadSize = 1024;
    const int packetSize = dataPayloadSize + 12;
    char ucBuffer[packetSize] = {0};
    char* byArrPkt = nullptr;
    Proto protocol;

    qint64 numByReadDone = 0;
    qint64 read_req_size = _iDataSize;
    int updateProgressbarCount = 0;
    abort = false;
    while (_iDataSize > 0 && !abort) {
        qint64 chunkToRequest = qMin(_iDataSize, static_cast<qint64>(dataPayloadSize));
        int pktLen = protocol.mPktBulkRead(startAddress, static_cast<int>(chunkToRequest), &byArrPkt);
        //Log::printHexRecvBuffer(byArrPkt,16);
        eth10G->sendData(byArrPkt, pktLen, eth10G->RemoteIP.toStdString(), eth10G->Port);

        qint64 sizeReceived = 0;
        while ((sizeReceived < chunkToRequest) && !abort) {

            std::string senderip;
            uint16_t senderPort;
            sizeReceived = eth10G->receiveData(ucBuffer, packetSize, senderip, senderPort);
            if (sizeReceived >= 12)
            {
                protocol.m_nPacketLength = protocol.mPktParseBulkRead(ucBuffer);
                int payloadLen = protocol.m_nPacketLength - 12;
                if (payloadLen > 0)
                {
                    fwrite(&ucBuffer[12], 1, payloadLen, pFile);
                    fflush(pFile);
                    bytesTransferred += payloadLen;
                    _iDataSize -= payloadLen;
                }
            }
            if ((updateProgressbarCount++ >= AFTER_NUMBER_OF_PKT) && !abort) {
                int percentageComplete = static_cast<int>((bytesTransferred * 100.0) / read_req_size);
                //emit progressUpdated(percentageComplete);
                updateProgressbarCount = 0;
            }
        }

        numByReadDone += sizeReceived;
        //startAddress += static_cast<unsigned int>(sizeReceived);
        //LOG_INFO("Chunk received: %lld bytes, Remaining: %lld, PacketLength: %d", sizeReceived, _iDataSize, protocol.m_nPacketLength);
    }

    if (numBytesRdSuccess)
        *numBytesRdSuccess = numByReadDone;

    fclose(pFile);
    throughputLog.write(QTime::currentTime().toString("hh:mm:ss:zzz").toUtf8() + '\n');
    throughputLog.close();

    LOG_INFO("[FileTransferAgent] Read complete: %lld bytes read", numByReadDone);
    LOG_INFO(abort ? "File transmission aborted" : "File transmission complete.");
    emit transferComplete(eReadDone);
    QFileInfo fileinfo(_sFilePath);
    if(fileinfo.exists() && fileinfo.isFile()){
        LOG_INFO("Steaming bin Size:%ld",fileinfo.size());
    }
    LOG_INFO("FileTransferAgent::StreamReadFileBulkEth10G <EXIT>");
    return 0;
}

int FileTransferAgent::GetTotalbyte(){
    return TransferReqSize;
}
int FileTransferAgent::GetTransferBbyte(){
    return bytesTransferred;
}

int FileTransferAgent::WriteFileBulk01G(unsigned startAddress, qint64* numBytesRdSuccess)
{
    LOG_TO_FILE("FileTransferAgent::WriteFileBulk01G <ENTER>");

    // --------- Per-call state reset (IMPORTANT for repeated calls) ----------
    if (numBytesRdSuccess) {
        *numBytesRdSuccess = 0;
    }

    bytesTransferred       = 0;
    TransferReqSize        = 0;
    abort                  = false;
    int updateProgressbarCount = 0;

    // ------------------------------------------------------------------------
    // Validate file
    // ------------------------------------------------------------------------
    if (!QFile::exists(_sFilePath)) {
        LOG_ERROR("File not found: %s", _sFilePath.toUtf8().constData());
        return -1;
    }

    QFile file(_sFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR("File open failed: %s", file.errorString().toUtf8().constData());
        return -2;
    }

    // ------------------------------------------------------------------------
    // Get socket instance
    // ------------------------------------------------------------------------
    ethPL01G = EthernetSocketPL1G::getInstance();
    if (!ethPL01G) {
        LOG_ERROR("Error: Eth01G unavailable");
        file.close();
        return -3;
    }
    // ------------------------------------------------------------------------
    // Decide how many bytes to send this call
    // ------------------------------------------------------------------------
    const qint64 fileSize = file.size();

    // If _iDataSize is 0 or larger than actual file, just send full fileSize
    const qint64 totalSize =   (_iDataSize > 0 && _iDataSize <= fileSize) ? _iDataSize : fileSize;

    qint64 remainingSize = totalSize;
    TransferReqSize      = totalSize;    // for UI / logging

    LOG_INFO("=========================================================================");
    LOG_INFO("FileWrite01G:: startAddress:%u size:%lld Filename:%s",  startAddress,  static_cast<long long>(totalSize), _sFilePath.toUtf8().constData());
    LOG_INFO("=========================================================================");

    // ------------------------------------------------------------------------
    // Main send loop
    // ------------------------------------------------------------------------
    while (remainingSize > 0 && !abort) {

        // Decide chunk size
        const qint64 chunkSize64 =  qMin<qint64>(remainingSize, static_cast<qint64>(MAX_BYTES_WRITE_AT_ONCE));

        const int chunkSize = static_cast<int>(chunkSize64);

        QByteArray chunk = file.read(chunkSize);

        if (chunk.isEmpty()) {
            if (file.error() != QFile::NoError) {
                LOG_ERROR("Read error: %s", file.errorString().toUtf8().constData());
                file.close();
                // numBytesRdSuccess already 0 or partial
                return -4;
            }

            LOG_INFO("End of file reached earlier than expected. " "remainingSize=%lld", static_cast<long long>(remainingSize));
            // Safety: stop outer loop as well so we don't spin forever
            remainingSize = 0;
            break;
        }

        // --------------------------------------------------------------------
        // Build protocol packet
        // --------------------------------------------------------------------
        Proto protocol;
        char* packetData = nullptr;

        int packetLen = protocol.mPktBulkWrite(startAddress, chunk.data(), chunk.size(), &packetData);
        if (!packetData || packetLen <= 0) {
            LOG_ERROR("Packet creation failed for chunk at addr: %u", startAddress);
            delete[] packetData;   // in case protocol allocated but length invalid
            file.close();
            return -5;
        }
        // --------------------------------------------------------------------
        // Send over 01G socket
        // --------------------------------------------------------------------
        if (!ethPL01G->sendData(packetData, packetLen)) {
            LOG_ERROR("sendData(01G) failed, packetLen=%d", packetLen);
            delete[] packetData;
            abort = true;
            break;   // exit loop; we will log below
        }

        delete[] packetData;
        // --------------------------------------------------------------------
        // Bookkeeping
        // --------------------------------------------------------------------
        remainingSize    -= chunk.size();     // logical bytes from user/file
        bytesTransferred += chunk.size();

        // If your protocol expects incremental address, do it here
        // startAddress += chunk.size();

        // Progress every AFTER_NUMBER_OF_PKT packets
        if (++updateProgressbarCount >= AFTER_NUMBER_OF_PKT && !abort) {
            int percentageComplete =  static_cast<int>((bytesTransferred * 100.0) / totalSize);
            LOG_INFO("percentageComplete %ld\n",percentageComplete);
            emit progressUpdated(percentageComplete);
            updateProgressbarCount = 0;
        }
        LOG_INFO("bytesTransferred:%lld remainingSize:%lld", static_cast<long long>(bytesTransferred), static_cast<long long>(remainingSize));
    }

    file.close();

    // ------------------------------------------------------------------------
    // Final status & outputs
    // ------------------------------------------------------------------------
    if (numBytesRdSuccess) {
        *numBytesRdSuccess = bytesTransferred;
    }

    if (!abort) {
        LOG_INFO("File transmission complete. Total bytes: %lld", static_cast<long long>(bytesTransferred));
    } else {
        LOG_INFO("File transmission aborted. Bytes sent: %lld", static_cast<long long>(bytesTransferred));
    }

    emit transferComplete(eWriteDone);
    LOG_INFO("FileTransferAgent::WriteFileBulk01G <EXIT>");

    // Return error code only if aborted or other error occurred
    return abort ? -6 : 0;
}



int FileTransferAgent::WriteFileBulk10G(unsigned startAddress,qint64* numBytesRdSuccess){

    LOG_INFO("FileTransferAgent::WriteFileBulk10G <ENTER>");
    constexpr int BUFFERED_PACKETS_SIZE = 0x10A000; // 1MB + 40KB
    if (!QFile::exists(_sFilePath)) {
        LOG_ERROR("File not found: %s", _sFilePath.toUtf8().constData());
        return -1;
    }
    QFile file(_sFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR("File open failed: %s", file.errorString().toUtf8().constData());
        return -2;
    }
    eth10G = EthernetSocket10G::getInstance();
    if(eth10G == nullptr){
        LOG_ERROR("Error: Eth10G not initilized !!!!!!");
        return -1;
    }
    char ucBuffer[1516] = {0};
    const unsigned int totalSize = _iDataSize;//file.size();
    unsigned int remainingSize = _iDataSize;//file.size();
    TransferReqSize  = file.size();
    int updateProgressbarCount = 0;
    bytesTransferred = 0;
    LOG_INFO("=========================================================================");
    LOG_INFO("FileWrite:: startAddress:%u size:%u Filename:%s", startAddress, totalSize, _sFilePath.toUtf8().constData());
    LOG_INFO("==========================================================================");
    abort = false;

    while ((remainingSize > 0) && !abort)
    {
        int bufferedSize = 0;
        while ((bufferedSize < (BUFFERED_PACKETS_SIZE - (MAX_BYTES_WRITE_AT_ONCE + 12)) && remainingSize > 0 ) && !abort) {
            int chunkSize = qMin(remainingSize, static_cast<unsigned int>(MAX_BYTES_WRITE_AT_ONCE));
            QByteArray chunk = file.read(chunkSize);

            if (chunk.isEmpty()) {
                if (file.error() != QFile::NoError) {
                    LOG_ERROR("Read error: %s", file.errorString().toUtf8().constData());
                    file.close();
                    return -3;
                }
                LOG_INFO("End of file reached.");
                break;
            }
            Proto protocol;
            char* packetData = nullptr;
            int packetLen = protocol.mPktBulkWrite(startAddress, chunk.data(), chunk.size(), &packetData);
            if (!packetData || packetLen <= 0) {
                LOG_ERROR("Packet creation failed for chunk at addr: %u", startAddress);
                file.close();
                return -4;
            }
            if(eth10G->sendData(packetData, packetLen, eth10G->RemoteIP.toStdString(),eth10G->Port)){
                //Read and discard the packet.
                //std::string senderip;
                //uint16_t senderPort;
                //int sizeReceived;
                //sizeReceived = eth10G->receiveData(ucBuffer, 1516, senderip, senderPort);
            }
            delete[] packetData;
            //startAddress += chunk.size();
            remainingSize -= chunk.size();
            bufferedSize += chunk.size();
            bytesTransferred += chunk.size();
            if((updateProgressbarCount++ < AFTER_NUMBER_OF_PKT) && !abort){
                int percentageComplete = static_cast<int>((bytesTransferred * 100.0) / totalSize);
                emit progressUpdated(percentageComplete);
                updateProgressbarCount = 0;
            }
        }
        LOG_INFO("bytesTransferred:%d remainingSize:%d ",bytesTransferred,remainingSize);
    }
    file.close();
    if(!abort)LOG_INFO("File transmission complete.");
    else LOG_INFO("File transmission aborted");
    emit transferComplete(eWriteDone);
    LOG_INFO("FileTransferAgent::WriteFileBulk10G <EXIT>");
    return 0;
}

