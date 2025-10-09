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
#include "proto.h"
#include "AvrRegAddrDef.h"

enum iface { eNONE,eETHPS1G,eETHPL1G, eETH10G, eSERIAL,ePLSERIAL, ePCIe };
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

// SPI Control Interface
void SpiCtrlWriteReg(iface deviceType, uint32_t offset, uint32_t value);
void WriteSpiSynth(iface deviceType, uint32_t address, uint32_t data);
uint32_t SpiCtrlReadReg (iface deviceType, uint32_t uiAddr);

// DDR3 Test Data Generator/Checker
void     Ddr3TdgWriteReg(iface deviceType, uint32_t offset, uint32_t value);
uint32_t Ddr3TdgReadReg (iface deviceType, uint32_t offset);

// DDR3 Read/Write Adapter
void     Ddr3RwWriteReg (iface deviceType, uint32_t offset, uint32_t value);
uint32_t Ddr3RwReadReg  (iface deviceType, uint32_t offset);

// DAC Interface
void    DacWriteReg(iface deviceType, uint32_t uiAddr, uint32_t value);
void    SpiDacWrite(iface deviceType, uint32_t Address, uint32_t Data, uint32_t sel);
uint32_t DacReadReg(iface deviceType, uint32_t uiAddr);
uint32_t SpiDacRead(iface deviceType, uint32_t Address, uint32_t sel);


// DAC0 DDR3 Adapter
void     Dac0AdaWriteReg(iface deviceType, uint32_t offset, uint32_t value);
uint32_t Dac0AdaReadReg (iface deviceType, uint32_t offset);

// DAC1 DDR3 Adapter
void     Dac1AdaWriteReg(iface deviceType, uint32_t offset, uint32_t value);
uint32_t Dac1AdaReadReg (iface deviceType, uint32_t offset);

// LVDS Interface
void     LvdsIfWriteReg (iface deviceType, uint32_t offset, uint32_t value);
uint32_t LvdsIfReadReg  (iface deviceType, uint32_t offset);

// SPI Flash Interface
void     SpiFlashWriteReg(iface deviceType, uint32_t offset, uint32_t value);
uint32_t SpiFlashReadReg (iface deviceType, uint32_t offset);

// Manufacturing Ctrl & Status (read-only)
uint32_t ManufReadReg   (iface deviceType, uint32_t offset);

// I2C Slave Control
void     I2cSlvWriteReg (iface deviceType, uint32_t offset, uint32_t value);
uint32_t I2cSlvReadReg  (iface deviceType, uint32_t offset);

// Common SW Interface Registers
void     IfCommonWriteReg(iface deviceType, uint32_t offset, uint32_t value);
uint32_t IfCommonReadReg (iface deviceType, uint32_t offset);

// Clock & Reset Control
void     ClkRstWriteReg (iface deviceType, uint32_t offset, uint32_t value);
uint32_t ClkRstReadReg  (iface deviceType, uint32_t offset);

}

#endif // UTILS_H
