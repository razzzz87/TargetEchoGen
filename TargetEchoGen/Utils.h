#ifndef UTILS_H
#define UTILS_H
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>
#include <QByteArray>
#include <QWidget>
#include <QString>
#include <cstdarg>
#include <QLineEdit>
#include "log.h"
#include "ethernetsocket.h"
#include "ethernetsocket10G.h"
#include "uartserial.h"
#include "Proto.h"

enum iface { eNONE,eETH1G, eETH10G, eSERIAL, ePCIe };
enum TransferMode { SendBulk, ReceiveBulk, Streaming };

enum eXferDir {
    eWrite,   // Data sent from host to target
    eStream,
    eRead    // Data received from target to host
};

struct stFileReadWriteConf{
    uint iFileSize;
    QString sFilePath;
    iface eInterface;
    eXferDir _Dir;
};


namespace Utils{

inline UartSerial* serial = nullptr;
inline EthernetSocket* eth1G = nullptr;
inline EthernetSocket10G* eth10G = nullptr;
inline QDialog* progressDialog = nullptr;

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

inline uint32_t isBitSet(uint32_t value, int pos) {
    return (value >> pos) & 1U;
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

// inline void FileReadWriteSetup(iface deviceType, uint iFileSize, const QString& sFilePath, eXferDir dir)
// {
//     LOG_INFO("[Utils::FileReadWriteSetup] <ENTER>");
//     char* byArrPkt = nullptr;
//     Proto protocolobj;

//     switch (deviceType)
//     {
//     case iface::eSERIAL:
//         if (!serial) {
//             LOG_TO_FILE("ERROR: Serial pointer is null.");
//             return;
//         }
//         break;

//     case iface::eETH1G:
//     case iface::eETH10G:
//     {
//         stFileReadWriteConf Cnf;
//         Cnf.iFileSize = iFileSize;
//         Cnf.sFilePath = sFilePath;
//         Cnf.eInterface = deviceType;
//         Cnf._Dir = dir;

//         if (setupTransferAgent) {
//             setupTransferAgent->configure(Cnf);
//             if (deviceType == iface::eETH10G) {
//                 setupTransferAgent->start();
//                 if (progressDialog) progressDialog->show();
//             }
//         } else {
//             LOG_TO_FILE("ERROR: TransferAgent pointer is null.");
//         }
//         break;
//     }

//     case iface::ePCIe:
//         LOG_INFO("PCIe interface not implemented.");
//         break;

//     default:
//         LOG_INFO("No valid interface selection");
//     }

//     delete[] byArrPkt;
//     LOG_INFO("[Utils::FileReadWriteSetup] <EXIT>");
// }

inline void readRegisterValue(iface deviceType, QLineEdit* lineEditAddr, QLineEdit* lineEditVal)
{
    LOG_TO_FILE("[Utils::readRegisterValue] <ENTER>");
    char* byArrPkt = nullptr;
    char ByteArr64BitPakt[64] = {0};
    bool ok;
    Proto protocolobj;

    uint addr = lineEditAddr->text().toUInt(&ok, 16);
    if (!ok) {
        LOG_TO_FILE("ERROR: Invalid address format.");
        return;
    }

    int pktLen = protocolobj.mPktRegRead(addr, &byArrPkt);

    switch (deviceType)
    {
    case iface::eSERIAL:
        if (!serial) {
            LOG_TO_FILE("ERROR: Serial pointer is null.");
            return;
        }
        if (serial->sendData(byArrPkt, pktLen) &&
            serial->receiveData(ByteArr64BitPakt, pktLen))
        {
            int reg_val = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
            lineEditVal->setText(QString("%1").arg(reg_val, 8, 16, QChar('0')).toUpper());
            LOG_TO_FILE("REG_VAL:0x%08X", reg_val);
        }
        break;

    case iface::eETH1G:
        if (!eth1G) {
            LOG_TO_FILE("ERROR: Ethernet pointer is null.");
            return;
        }
        if (eth1G->sendData(byArrPkt, pktLen, eth1G->RemoteIP.toStdString(), eth1G->Port)) {
            std::string senderIp;
            uint16_t senderport;
            if (eth1G->receiveData(ByteArr64BitPakt, pktLen, senderIp, senderport)) {
                int reg_val = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
                lineEditVal->setText(QString("%1").arg(reg_val, 8, 16, QChar('0')).toUpper());
                LOG_TO_FILE("REG_VAL:0x%08X", reg_val);
            }
        }
        break;

    case iface::eETH10G:
        if (!eth10G) {
            LOG_TO_FILE("ERROR: Ethernet pointer is null.");
            return;
        }
        if (eth10G->sendData(byArrPkt, pktLen, eth10G->RemoteIP.toStdString(), eth10G->Port)) {
            std::string senderIp;
            uint16_t senderport;
            if (eth10G->receiveData(ByteArr64BitPakt, pktLen, senderIp, senderport)) {
                int reg_val = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
                lineEditVal->setText(QString("%1").arg(reg_val, 8, 16, QChar('0')).toUpper());
                LOG_TO_FILE("REG_VAL:0x%08X", reg_val);
            }
        }
        break;

    case iface::ePCIe:
        LOG_TO_FILE("PCIe interface not implemented.");
        break;

    default:
        LOG_TO_FILE("No valid interface selection");
    }

    delete[] byArrPkt;
    LOG_TO_FILE("[Utils::readRegisterValue] <EXIT>");
}
// Add more as needed...
}
#endif // UTILS_H
