#include "FileTransferAgent.h"
#include "log.h"
#include <QDebug>
#include <QtGlobal>
#include <QFileInfo>

void FileTransferAgent::run()
{
    LOG_INFO("FileTransferAgent::run() <ENTER>");
    LOG_DEBUG("eInterFace:%d",_eInterFace);
    if(_eInterFace == eETH1G){

        if(this->_Dir == eWrite){
             LOG_DEBUG("FileTransferAgent::run()eWrite ");
        }
        else{
            LOG_DEBUG("FileTransferAgent::run() eRead ");
            ReadFileBulkEth01G(0x40000,NULL);
        }
    }
    else if (_eInterFace == eETH10G){

        if(_Dir == eWrite){
             LOG_DEBUG("eETH10G FileTransferAgent::run() eWrite ");
            WriteFileBulk10G(0x4200,NULL);
        }
        else if(_Dir == eStream){
            StreamReadFileBulkEth10G(0x4200,NULL);
        }
        else{
            LOG_DEBUG("eETH10G FileTransferAgent::run() eRead ");
            ReadFileBulkEth10G(0x4200,NULL);
        }
    }
    else if(_eInterFace == eSERIAL){

        if(_Dir == eWrite){
        }
        else{
            LOG_DEBUG("FileTransferAgent::run() eRead ");
            ReadFileBulkEth10G(0x40000,NULL);
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

    if(deviceType == eETH1G){
        this->eth0 = EthernetSocket::getInstance();
    }
    else if(deviceType == eETH10G){
        this->eth10G = EthernetSocket10G::getInstance();
    }else{

    }
}
int FileTransferAgent::ReadFileBulkEth01G(unsigned int startAddress, qint64* numBytesRdSuccess)
{
    LOG_TO_FILE("FileTransferAgent::ReadFileBulkEth01G <ENTER>");
    QFile throughputLog("test_10G.log");
    throughputLog.open(QIODevice::Append);
    throughputLog.write(QTime::currentTime().toString("hh:mm:ss:zzz").toUtf8() + '\n');
    throughputLog.write(_sFilePath.toUtf8() + '\n');

    FILE* pFile = fopen(_sFilePath.toUtf8().constData(), "wb");
    if (!pFile || !eth0->getConnStatus()) {
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
    qint64 read_req_size = _iDataSize;

    while (_iDataSize > 0 && !abort) {
        qint64 chunkToRequest = qMin(_iDataSize, static_cast<qint64>(maxChunkSize));

        int pktLen = protocol.mPktBulkRead(startAddress, static_cast<int>(chunkToRequest), &byArrPkt);
        eth0->sendData(byArrPkt, pktLen, eth0->RemoteIP.toStdString(), eth0->Port);

        qint64 sizeReceived = 0;
        while ((sizeReceived < chunkToRequest) && !abort)
        {
            std::string senderip;
            uint16_t senderPort;
            int nDatagram = 0;
            if(eth0->receiveData(ucBuffer, pktLen,senderip,senderPort)){
                //Asumming if recived sucessfully it will
                nDatagram = pktLen;
            }
            if (nDatagram > 12)
            {
                protocol.mPktParseBulkRead(ucBuffer);
                if (protocol.m_nPacketLength != 0) {
                    fwrite(&ucBuffer[12], 1, protocol.m_nPacketLength - 12, pFile);
                    fflush(pFile);
                    sizeReceived += protocol.m_nPacketLength - 12;
                    bytesTransferred += protocol.m_nPacketLength - 12;
                    _iDataSize -= (protocol.m_nPacketLength - 12); // Header offset
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
        LOG_TO_FILE("Chunk received: %lld bytes, Remaining: %lld m_nPacketLength:%d", sizeReceived, _iDataSize,protocol.m_nPacketLength);
    }
    if (numBytesRdSuccess)
        *numBytesRdSuccess = numByReadDone;

    fclose(pFile);
    delete[] rDevBuf;
    throughputLog.write(QTime::currentTime().toString("hh:mm:ss:zzz").toUtf8() + '\n');
    throughputLog.close();

    LOG_TO_FILE("[FileTransferAgent] Upload complete: %lld bytes read", numByReadDone);
    LOG_TO_FILE(abort ? "File transmission aborted" : "File transmission complete.");

    emit transferComplete();  // ✅ Progress dialog cleanup
    LOG_TO_FILE("FileTransferAgent::ReadFileBulkEth01G <EXIT>");
    return 0;
}

int FileTransferAgent::ReadFileBulkEth10G(unsigned int startAddress, qint64* numBytesRdSuccess)
{
    LOG_TO_FILE("FileTransferAgent::ReadFileBulkEth10G <ENTER>");
    LOG_INFO("ReadSize:%d,FilePath:%s",_iDataSize,_sFilePath.toStdString().c_str());
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
        //LOG_TO_FILE("Chunk received: %lld bytes, Remaining: %lld, PacketLength: %d", sizeReceived, _ReadSize, protocol.m_nPacketLength);
    }

    if (numBytesRdSuccess)
        *numBytesRdSuccess = numByReadDone;

    fclose(pFile);
    throughputLog.write(QTime::currentTime().toString("hh:mm:ss:zzz").toUtf8() + '\n');
    throughputLog.close();

    LOG_TO_FILE("[FileTransferAgent] Read complete: %lld bytes read", numByReadDone);
    LOG_TO_FILE(abort ? "File transmission aborted" : "File transmission complete.");
    emit transferComplete();
    LOG_TO_FILE("FileTransferAgent::ReadFileBulkEth10G <EXIT>");
    return 0;
}

int FileTransferAgent::StreamReadFileBulkEth10G(unsigned int startAddress, qint64* numBytesRdSuccess)
{
    LOG_TO_FILE("FileTransferAgent::StreamReadFileBulkEth10G <ENTER>");
    LOG_INFO("ReadSize:%d,FilePath:%s",_iDataSize,_sFilePath.toStdString().c_str());
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
        //LOG_TO_FILE("Chunk received: %lld bytes, Remaining: %lld, PacketLength: %d", sizeReceived, _iDataSize, protocol.m_nPacketLength);
    }

    if (numBytesRdSuccess)
        *numBytesRdSuccess = numByReadDone;

    fclose(pFile);
    throughputLog.write(QTime::currentTime().toString("hh:mm:ss:zzz").toUtf8() + '\n');
    throughputLog.close();

    LOG_TO_FILE("[FileTransferAgent] Read complete: %lld bytes read", numByReadDone);
    LOG_TO_FILE(abort ? "File transmission aborted" : "File transmission complete.");
    emit transferComplete();
    QFileInfo fileinfo(_sFilePath);
    if(fileinfo.exists() && fileinfo.isFile()){
        LOG_INFO("Steaming bin Size:%ld",fileinfo.size());
    }
    LOG_TO_FILE("FileTransferAgent::StreamReadFileBulkEth10G <EXIT>");
    return 0;
}

int FileTransferAgent::GetTotalbyte(){
    return TransferReqSize;
}
int FileTransferAgent::GetTransferBbyte(){
    return bytesTransferred;
}
#if 0
int FileTransferAgent::sendFileBulkPS01G(unsigned int startAddress, unsigned int size)
{
    LOG_TO_FILE("FileTransferAgent::sendFileBulkPS01G <ENTER>");
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
            if(eth0 != NULL){
                eth0->sendData(packetData, packetLen, eth0->RemoteIP.toStdString(),eth0->Port);
            }
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
        LOG_TO_FILE("bytesTransferred:%d remainingSize:%d ",bytesTransferred,remainingSize);
    }
    file.close();
    if(!abort)LOG_TO_FILE("File transmission complete.");
    else LOG_TO_FILE("File transmission aborted");
    emit close_progress_pop();
    LOG_TO_FILE("FileTransferAgent::sendFileBulkPS01G <EXIT>");
    return 0;
}
#endif

int FileTransferAgent::WriteFileBulk10G(unsigned startAddress,qint64* numBytesRdSuccess){

    LOG_TO_FILE("FileTransferAgent::sendFileBulkPS01G <ENTER>");
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
        LOG_TO_FILE("bytesTransferred:%d remainingSize:%d ",bytesTransferred,remainingSize);
    }
    file.close();
    if(!abort)LOG_TO_FILE("File transmission complete.");
    else LOG_TO_FILE("File transmission aborted");
    emit transferComplete();
    LOG_TO_FILE("FileTransferAgent::sendFileBulkPS01G <EXIT>");
    return 0;
}

