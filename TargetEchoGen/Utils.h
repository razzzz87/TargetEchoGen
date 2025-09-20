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
#include "ethernetsocketpl1g.h"
#include "uartserial.h"
#include "Proto.h"

enum iface { eNONE,eETHPS1G,eETHPL1G, eETH10G, eSERIAL, ePCIe };
enum TransferMode { SendBulk, ReceiveBulk, Streaming };

enum eXferDir {
    eWrite,   // Data sent from host to target
    eStream,  //
    eRead     // Data received from target to host
};

struct stFileReadWriteConf{
    uint iFileSize;
    QString sFilePath;
    iface eInterface;
    eXferDir _Dir;
};

enum class ControlBit : uint8_t {
    EnableFeatureX = 0,
    ResetModule    = 1,
    StartTransfer  = 2,
    // Add more as needed
};

enum class BitState : bool {
    Disable = false,
    Enable  = true
};
namespace Utils {

uint32_t setBit(uint32_t& value, int pos);
uint64_t setBit64(uint64_t& value, int pos);

uint32_t clearBit(uint32_t& value, int pos);
uint64_t clearBit64(uint64_t& value, int pos);

bool isBitSet(uint32_t value, int pos);
uint32_t setBits(uint32_t value, int start, int end);
uint32_t clearBits(uint32_t value, int start, int end);

uint32_t setValueInBits19to12(uint32_t reg, uint8_t value);
uint16_t extractBits15to0(uint32_t value);
void setControlBit(uint32_t& reg_val, ControlBit bit, BitState state);
void readRegisterValue(iface deviceType, QLineEdit* lineEditAddr, QLineEdit* lineEditVal);
uint readRegisterValue(iface deviceType,uint addr);
void RegisterWrite(iface deviceType, uint iaddr, uint ival);
}
#endif // UTILS_H
