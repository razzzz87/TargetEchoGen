#include "Utils.h"
#include "AvrRegAddrDef.h"
//#include "RegAccessWrappers.h"
namespace Utils{

inline UartSerial* serial = nullptr;
inline EthernetSocket* eth1G = nullptr;
inline EthernetSocketPL1G* ethPl1G = nullptr;
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
inline void setControlBit(uint32_t& reg_val, ControlBit bit, BitState state) {
    uint8_t pos = static_cast<uint8_t>(bit);
    if (state == BitState::Enable)
        reg_val |= (1 << pos);
    else
        reg_val &= ~(1 << pos);
}

void readRegisterValue(iface deviceType, QLineEdit* lineEditAddr, QLineEdit* lineEditVal)
{
    bool ok;

    LOG_INFO("[Utils::readRegisterValue] <ENTER> Addr:0x%08X",lineEditAddr->text().toUInt(&ok, 16));
    char* byArrPkt = nullptr;
    char ByteArr64BitPakt[64] = {0};
    Proto protocolobj;

    uint addr = lineEditAddr->text().toUInt(&ok, 16);
    if (!ok) {
        LOG_ERROR("Invalid address format.");
        return;
    }

    int pktLen = protocolobj.mPktRegRead(addr, &byArrPkt);

    switch (deviceType)
    {
    case iface::eSERIAL:
        if (!serial) {
            LOG_ERROR("ERROR: Serial pointer is null.");
            return;
        }
        if (serial->sendData(byArrPkt, pktLen) &&
            serial->receiveData(ByteArr64BitPakt, pktLen))
        {
            int reg_val = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
            lineEditVal->setText(QString("%1").arg(reg_val, 8, 16, QChar('0')).toUpper());
            LOG_INFO("REG_VAL:0x%08X", reg_val);
        }
        break;

    case iface::eETHPL1G:
        ethPl1G = EthernetSocketPL1G::getInstance();
        if (!ethPl1G) {
            LOG_ERROR("Ethernet pointer is null.");
            return;
        }
        if (ethPl1G->sendData(byArrPkt, pktLen)) {
            int RecvByte;
            if (ethPl1G->receiveData(ByteArr64BitPakt, pktLen, RecvByte)) {
                int reg_val = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
                lineEditVal->setText(QString("%1").arg(reg_val, 8, 16, QChar('0')).toUpper());
                LOG_TO_FILE("REG_VAL:0x%08X", reg_val);
            }
        }
        break;

    case iface::eETH10G:
        if (!eth10G) {
            LOG_ERROR("Ethernet pointer is null.");
            return;
        }
        if (eth10G->sendData(byArrPkt, pktLen, eth10G->RemoteIP.toStdString(), eth10G->Port)) {
            std::string senderIp;
            uint16_t senderport;
            if (eth10G->receiveData(ByteArr64BitPakt, pktLen, senderIp, senderport)) {
                int reg_val = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
                lineEditVal->setText(QString("%1").arg(reg_val, 8, 16, QChar('0')).toUpper());
                LOG_INFO("REG_VAL:0x%08X", reg_val);
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
uint readRegisterValue(iface deviceType,uint addr)
{
    LOG_INFO("[Utils::readRegisterValue] <ENTER> Addr:0x%08X",addr);
    char* byArrPkt = nullptr;
    uint reg_val = -20;
    char ByteArr64BitPakt[64];
    Proto protocolobj;
    int pktLen = protocolobj.mPktRegRead(addr, &byArrPkt);
    switch (deviceType)
    {
    case iface::eSERIAL:
    {
        serial = UartSerial::getInstance();
        if (!serial)
        {
            LOG_ERROR("ERROR: Serial pointer is null.");
            return -1;
        }
        if(!serial->sendData(byArrPkt, pktLen))
        {
            LOG_ERROR("Sent filed!!!<Serial>");
        }
        else
        {
            serial->sendData(byArrPkt, pktLen);
            if(serial->receiveData(ByteArr64BitPakt, pktLen)){
                int reg_val = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
                LOG_INFO("REG_VAL:0x%08X",reg_val);
            }
        }
        break;
    }
    case iface::eETHPL1G:
    {
        ethPl1G = EthernetSocketPL1G::getInstance();
        if (!ethPl1G) {
            LOG_ERROR("Ethernet pointer is null.");
            return -1;
        }
        if (ethPl1G->sendData(byArrPkt, pktLen)) {
            int RecvByte;
            if (ethPl1G->receiveData(ByteArr64BitPakt, pktLen, RecvByte)) {
                int reg_val = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
                LOG_TO_FILE("REG_VAL:0x%08X", reg_val);
            }
        }
        break;
    }
    break;
    case iface::ePCIe:
        break;
    case iface::eETH10G:
    {
        eth10G = EthernetSocket10G::getInstance();
        if (!eth10G) {
            LOG_TO_FILE("ERROR: Ethernet pointer is null.");
            return -1;
        }
        if(!eth10G->sendData(byArrPkt,pktLen,eth10G->RemoteIP.toStdString(),eth10G->Port)){
            LOG_TO_FILE("Sent filed!!!<eth10G>");
        }
        {
            char ByteArr64BitPakt[64]={0};
            std::string senderIp;
            uint16_t senderport;
            if(eth10G->receiveData(ByteArr64BitPakt,pktLen,senderIp,senderport))
            {
                reg_val = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
                LOG_TO_FILE("REG_VAL:0x%08X",reg_val);
            }else{
                LOG_TO_FILE("Receive filed!!!<eth10G>");
            }
        }
    }
    break;
    default:
        LOG_TO_FILE("No valid interface selection");
    }
    delete byArrPkt;
    LOG_INFO("[Utils::readRegisterValue] <EXIT> Addr:0x%08X RegVal:%X",addr,reg_val);
    return reg_val;
}
void RegisterWrite(iface deviceType, uint iaddr, uint ival)
{
    LOG_INFO("[Utils::RegisterWrite] <ENTER> Addr:0x%08X Val:0x%08X",iaddr,ival);
    char* byArrPkt = nullptr;

    Proto protocolobj;
    int pktLen = protocolobj.mPktRegWrite(iaddr, ival, &byArrPkt);
    switch (deviceType)
    {
    case iface::eSERIAL:
    {
        serial = UartSerial::getInstance();
        if (!serial) {
            LOG_ERROR("ERROR: Serial pointer is null.");
            return;
        }
        if(!serial->sendData(byArrPkt, pktLen)){
            LOG_ERROR("Sent filed!!!<Serial>");
        }
        break;
    }

    case iface::eETHPL1G:
    {
        ethPl1G = EthernetSocketPL1G::getInstance();
        if (!ethPl1G) {
            LOG_ERROR("Ethernet pointer is null.");
            return;
        }
        if(!ethPl1G->sendData(byArrPkt,pktLen)){
            LOG_ERROR("Sent filed!!!<eth1G>");
        }
        {
            char ByteArr64BitPakt[64]={0};
            int RecvByte;
            if (ethPl1G->receiveData(ByteArr64BitPakt, pktLen, RecvByte)) {
                int reg_val = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
                LOG_INFO("REG_VAL:0x%08X", reg_val);
            }
        }
        break;
    }
    case eETH10G:
        eth10G = EthernetSocket10G::getInstance();
        if (!eth10G) {
            LOG_ERROR("Ethernet pointer is null.");
            return;
        }
        if(!eth10G->sendData(byArrPkt,pktLen,eth10G->RemoteIP.toStdString(),eth10G->Port)){
            LOG_ERROR("Sent filed!!!<eth1G>");
        }
        {
            char ByteArr64BitPakt[64]={0};
            std::string senderIp;
            uint16_t senderport;
            //Read and discard the packet
            if(eth10G->receiveData(ByteArr64BitPakt,pktLen,senderIp,senderport))
            {
                int reg_val = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
                LOG_INFO("RegVal:0x%08X",reg_val);

            }else{
                LOG_ERROR("Receive filed!!!<eth10G>");
            }
        }
        break;
    case ePCIe:
        break;
    case eNONE:
        break;
    case eETHPS1G:
        break;
    }
    LOG_INFO("[Utils::RegisterWrite] <EXIT>");
}
void SpiCtrlWriteReg(iface deviceType, uint32_t off, uint32_t v) {
    RegisterWrite(deviceType, AVR_SPI_CTRL_BASE_ADDR + off, v);
}
uint32_t SpiCtrlReadReg(iface deviceType, uint32_t off) {
    return readRegisterValue(deviceType, AVR_SPI_CTRL_BASE_ADDR + off);
}

void Ddr3TdgWriteReg(iface deviceType, uint32_t off, uint32_t v) {
    RegisterWrite(deviceType, AVR_DDR3_TDG_BASE_ADDR + off, v);
}
uint32_t Ddr3TdgReadReg(iface deviceType, uint32_t off) {
    return readRegisterValue(deviceType, AVR_DDR3_TDG_BASE_ADDR + off);
}

void Ddr3RwWriteReg(iface deviceType, uint32_t off, uint32_t v) {
    RegisterWrite(deviceType, AVR_DDR3_RW_ADAPTER_BASE_ADDR + off, v);
}
uint32_t Ddr3RwReadReg(iface deviceType, uint32_t off) {
    return readRegisterValue(deviceType, AVR_DDR3_RW_ADAPTER_BASE_ADDR + off);
}

void DacIfWriteReg(iface deviceType, uint32_t off, uint32_t v) {
    RegisterWrite(deviceType, AVR_DAC_INTERFACE_BASE_ADDR + off, v);
}
uint32_t DacIfReadReg(iface deviceType, uint32_t off) {
    return readRegisterValue(deviceType, AVR_DAC_INTERFACE_BASE_ADDR + off);
}

void Dac0AdaWriteReg(iface deviceType, uint32_t off, uint32_t v) {
    RegisterWrite(deviceType, AVR_DAC0_DDR3_ADAPTER_BASE_ADDR + off, v);
}
uint32_t Dac0AdaReadReg(iface deviceType, uint32_t off) {
    return readRegisterValue(deviceType, AVR_DAC0_DDR3_ADAPTER_BASE_ADDR + off);
}

void Dac1AdaWriteReg(iface deviceType, uint32_t off, uint32_t v) {
    RegisterWrite(deviceType, AVR_DAC1_DDR3_ADAPTER_BASE_ADDR + off, v);
}
uint32_t Dac1AdaReadReg(iface deviceType, uint32_t off) {
    return readRegisterValue(deviceType, AVR_DAC1_DDR3_ADAPTER_BASE_ADDR + off);
}

void LvdsIfWriteReg(iface deviceType, uint32_t off, uint32_t v) {
    RegisterWrite(deviceType, AVR_LVDS_INTERFACE_BASE_ADDR + off, v);
}
uint32_t LvdsIfReadReg(iface deviceType, uint32_t off) {
    return readRegisterValue(deviceType, AVR_LVDS_INTERFACE_BASE_ADDR + off);
}

void SpiFlashWriteReg(iface deviceType, uint32_t off, uint32_t v) {
    RegisterWrite(deviceType, AVR_SPI_FLASH_BASE_ADDR + off, v);
}
uint32_t SpiFlashReadReg(iface deviceType, uint32_t off) {
    return readRegisterValue(deviceType, AVR_SPI_FLASH_BASE_ADDR + off);
}

uint32_t ManufReadReg(iface deviceType, uint32_t off) {
    return readRegisterValue(deviceType, AVR_MANUF_CTRL_STATUS_BASE_ADDR + off);
}

void I2cSlvWriteReg(iface deviceType, uint32_t off, uint32_t v) {
    RegisterWrite(deviceType, AVR_I2C_SLAVE_CTRL_BASE_ADDR + off, v);
}
uint32_t I2cSlvReadReg(iface deviceType, uint32_t off) {
    return readRegisterValue(deviceType, AVR_I2C_SLAVE_CTRL_BASE_ADDR + off);
}

void IfCommonWriteReg(iface deviceType, uint32_t off, uint32_t v) {
    RegisterWrite(deviceType, AVR_INTERFACE_BASE_ADDR + off, v);
}
uint32_t IfCommonReadReg(iface deviceType, uint32_t off) {
    return readRegisterValue(deviceType, AVR_INTERFACE_BASE_ADDR + off);
}

void ClkRstWriteReg(iface deviceType, uint32_t off, uint32_t v) {
    RegisterWrite(deviceType, AVR_CLK_RST_CTRL_BASE_ADDR + off, v);
}
uint32_t ClkRstReadReg(iface deviceType, uint32_t off) {
    return readRegisterValue(deviceType, AVR_CLK_RST_CTRL_BASE_ADDR + off);
}

}

// // File: ExampleOneInterface.cpp
// // Demonstrates every RegAccess function using the eETHPL1G interface

// #include <cstdio>
// #include "iface.h"                        // enum iface { eNONE, eETHPS1G, eETHPL1G, eETH10G, eSERIAL, ePCIe };
// #include "RegisterAccess.h"               // RegAccess::… wrappers
// #include "AVR_SPI_Control_Reg.h"          // AVR_SPI_Control_Reg enum
// #include "AVR_DDR3_TDG_Reg.h"             // AVR_DDR3_TDG_Reg enum
// #include "AVR_DDR3_RW_Adapter_Reg.h"      // AVR_DDR3_RW_Adapter_Reg enum
// #include "AVR_DAC_Interface_Reg.h"        // AVR_DAC_Interface_Reg enum
// #include "AVR_DAC0_DDR3_Adapter_Reg.h"    // AVR_DAC0_DDR3_Adapter_Reg enum
// #include "AVR_DAC1_DDR3_Adapter_Reg.h"    // AVR_DAC1_DDR3_Adapter_Reg enum
// #include "AVR_LVDS_Interface_Reg.h"       // AVR_LVDS_Interface_Reg enum
// #include "AVR_SPI_Flash_Reg.h"            // AVR_SPI_FLASH_Reg enum
// #include "AVR_Manufacturing_Ctrl_Status_Reg.h"  // AVR_Manufacturing_Ctrl_Status_Reg enum
// #include "AVR_I2C_Slave_Control_Reg.h"    // AVR_I2C_Slave_Control_Reg enum
// #include "AVR_Interface_Reg.h"            // AVR_Interface_Reg enum
// #include "AVR_CLK_RST_CTRL_Reg.h"         // AVR_CLK_RST_CTRL_Reg enum

// using namespace RegAccess;

// int main()
// {
//     // Select the PHY-driven 1-Gigabit PL Ethernet interface
//     const iface dev = eETHPL1G;

//     // 1) SPI Control Interface
//     SpiCtrlWriteReg(dev, AVR_SPI_CONTROL_INSTR, 1);
//     SpiCtrlWriteReg(dev, AVR_SPI_CONTROL_INSTR, 0);
//     uint32_t spiCtrlStat = SpiCtrlReadReg(dev, AVR_SPI_CONTROL_STATUS);
//     std::printf("SPI-CTRL Status        = 0x%08X\n", spiCtrlStat);

//     // 2) DDR3 Test Data Generator/Checker
//     Ddr3TdgWriteReg(dev, AVR_DDR3_TDG_CONTROL, 1);
//     uint32_t tdgStat    = Ddr3TdgReadReg(dev, AVR_DDR3_TDG_STATUS);
//     std::printf("DDR3-TDG Status       = 0x%08X\n", tdgStat);

//     // 3) DDR3 Read/Write Adapter
//     Ddr3RwWriteReg(dev, AVR_DDR3_RW_ADAPTER_CONTROL, 1);
//     uint32_t rwCount    = Ddr3RwReadReg(dev, AVR_DDR3_RW_ADAPTER_COUNT);
//     std::printf("DDR3-RW Count         = %u\n", rwCount);

//     // 4) DAC Interface
//     DacIfWriteReg(dev, AVR_DAC_CONTROL, 0xAA);
//     uint32_t dacUnder    = DacIfReadReg(dev, AVR_DAC_UNDERFLOW_STATUS);
//     std::printf("DAC Underflow         = %u\n", dacUnder);

//     // 5) DAC0 DDR3 Adapter
//     Dac0AdaWriteReg(dev, AVR_DAC0_DDR3_ADAPTER_CONTROL, 1);
//     uint32_t dac0Size    = Dac0AdaReadReg(dev, AVR_DAC0_DDR3_ADAPTER_READ_SIZE);
//     std::printf("DAC0 Read Size        = %u\n", dac0Size);

//     // 6) DAC1 DDR3 Adapter
//     Dac1AdaWriteReg(dev, AVR_DAC1_DDR3_ADAPTER_CONTROL, 1);
//     uint32_t dac1Size    = Dac1AdaReadReg(dev, AVR_DAC1_DDR3_ADAPTER_READ_SIZE);
//     std::printf("DAC1 Read Size        = %u\n", dac1Size);

//     // 7) LVDS Interface
//     LvdsIfWriteReg(dev, AVR_LVDS_CONTROL, 1);
//     uint32_t lvdsStat    = LvdsIfReadReg(dev, AVR_LVDS_STATUS);
//     std::printf("LVDS Status           = 0x%X\n", lvdsStat);

//     // 8) SPI Flash Interface
//     SpiFlashWriteReg(dev, AVR_SPI_FLASH_INSTR, (1 << 0)); // Read pulse
//     uint32_t flashStat  = SpiFlashReadReg(dev, AVR_SPI_FLASH_STATUS);
//     std::printf("SPI-Flash Status      = 0x%X\n", flashStat);

//     // 9) Manufacturing Control & Status
//     uint32_t pcieLinkUp = ManufReadReg(dev, AVR_MANUF_PCIE_LINK_STATUS) & 0x1;
//     std::printf("PCIe Link Up          = %u\n", pcieLinkUp);

//     // 10) I2C Slave Control
//     uint32_t i2cCtrl    = (1 << 0)      // Write Data Valid pulse
//                        | (1 << 4);   // Message Size = 1
//     I2cSlvWriteReg(dev, AVR_I2C_MSG_CTRL, i2cCtrl);
//     uint32_t i2cData    = I2cSlvReadReg(dev, AVR_I2C_READ_DATA);
//     std::printf("I2C Read Data         = 0x%X\n", i2cData);

//     // 11) Common SW Interface
//     IfCommonWriteReg(dev, AVR_INTERFACE_SELECTION, 0x2); // USB
//     uint32_t ifaceSel   = IfCommonReadReg(dev, AVR_INTERFACE_SELECTION);
//     std::printf("Interface Selection   = %u\n", ifaceSel);

//     // 12) Clock & Reset Control
//     ClkRstWriteReg(dev, AVR_CLK_RST_CTRL_RESET_REGS, (1 << 9)); // Design Reset
//     uint32_t pllLocked  = ClkRstReadReg(dev, AVR_CLK_RST_CTRL_PLL_LOCK_STATUS) & 0x1;
//     std::printf("PLL Locked            = %u\n", pllLocked);

//     return 0;
// }
// int main()
// {
//     // 1) SPI Control Interface
//     //    Pulse Read Instruction, then poll status
//     SpiCtrlWriteReg(ePCIe, AVR_SPI_CONTROL_INSTR, 1);
//     SpiCtrlWriteReg(ePCIe, AVR_SPI_CONTROL_INSTR, 0);
//     uint32_t spiCtrlStatus = SpiCtrlReadReg(ePCIe, AVR_SPI_CONTROL_STATUS);
//     std::printf("SPI-CTRL Status = 0x%08X\n", spiCtrlStatus);

//     // 2) DDR3 Test Data Generator / Checker
//     //    Start generation, then read status
//     Ddr3TdgWriteReg(eETHPS1G, AVR_DDR3_TDG_CONTROL, 1);
//     uint32_t tdgStatus = Ddr3TdgReadReg(eETHPS1G, AVR_DDR3_TDG_STATUS);
//     std::printf("DDR3-TDG Status = 0x%08X\n", tdgStatus);

//     // 3) DDR3 Read/Write Adapter
//     //    Trigger transfer, then read actual count
//     Ddr3RwWriteReg(eETHPL1G, AVR_DDR3_RW_ADAPTER_CONTROL, 1);
//     uint32_t rwCount = Ddr3RwReadReg(eETHPL1G, AVR_DDR3_RW_ADAPTER_COUNT);
//     std::printf("DDR3-RW Count = %u\n", rwCount);

//     // 4) DAC Interface
//     //    Write control, read underflow status
//     DacIfWriteReg(eSERIAL, AVR_DAC_CONTROL, 0xAA);
//     uint32_t dacUnderflow = DacIfReadReg(eSERIAL, AVR_DAC_UNDERFLOW_STATUS);
//     std::printf("DAC Underflow = %u\n", dacUnderflow);

//     // 5) DAC0 DDR3 Adapter
//     //    Enable adapter, then read back page count
//     Dac0AdaWriteReg(ePCIe, AVR_DAC0_DDR3_ADAPTER_CONTROL, 1);
//     uint32_t dac0Pages = Dac0AdaReadReg(ePCIe, AVR_DAC0_DDR3_ADAPTER_READ_SIZE);
//     std::printf("DAC0 Read Size = %u\n", dac0Pages);

//     // 6) DAC1 DDR3 Adapter
//     //    Enable adapter, then read back page count
//     Dac1AdaWriteReg(ePCIe, AVR_DAC1_DDR3_ADAPTER_CONTROL, 1);
//     uint32_t dac1Pages = Dac1AdaReadReg(ePCIe, AVR_DAC1_DDR3_ADAPTER_READ_SIZE);
//     std::printf("DAC1 Read Size = %u\n", dac1Pages);

//     // 7) LVDS Interface
//     //    Configure LVDS, then read status
//     LvdsIfWriteReg(eETH10G, AVR_LVDS_CONTROL, 0x1);
//     uint32_t lvdsStatus = LvdsIfReadReg(eETH10G, AVR_LVDS_STATUS);
//     std::printf("LVDS Status = 0x%X\n", lvdsStatus);

//     // 8) SPI Flash Interface
//     //    Issue Bulk Erase, then read busy flag
//     SpiFlashWriteReg(eETHPS1G, AVR_SPI_FLASH_INSTR, (1 << 3));  // bulk erase bit
//     uint32_t flashBusy = SpiFlashReadReg(eETHPS1G, AVR_SPI_FLASH_STATUS) & 0x1;
//     std::printf("SPI Flash Busy = %u\n", flashBusy);

//     // 9) Manufacturing Control & Status
//     //    Read PCIe link status
//     uint32_t pcieLinkUp = ManufReadReg(ePCIe, AVR_MANUF_PCIE_LINK_STATUS) & 0x1;
//     std::printf("PCIe Link Up = %u\n", pcieLinkUp);

//     // 10) I2C Slave Control
//     //     Send a 1-byte write, then read back data
//     I2cSlvWriteReg(eSERIAL, AVR_I2C_MSG_CTRL,
//                    (1 << 0)      // write-pulse
//                        | (1 << 4)      // message size = 1
//                        | (0 << 12));   // code = write
//     uint32_t i2cData = I2cSlvReadReg(eSERIAL, AVR_I2C_READ_DATA);
//     std::printf("I2C Read Data = 0x%X\n", i2cData);

//     // 10) I2C Slave Control
//     //     Send a 1-byte write, then read back data
//     I2cSlvWriteReg(eSERIAL, AVR_I2C_MSG_CTRL,
//                    (1 << 0)      // write-pulse
//                        | (1 << 4)      // message size = 1
//                        | (0 << 12));   // code = write
//     uint32_t i2cData = I2cSlvReadReg(eSERIAL, AVR_I2C_READ_DATA);
//     std::printf("I2C Read Data = 0x%X\n", i2cData);

//     // 11) Common Interface Registers
//     //     Switch to USB interface (code = “10”)
//     IfCommonWriteReg(eETH10G, AVR_INTERFACE_SELECTION, 0x2);
//     uint32_t ifaceSel = IfCommonReadReg(eETH10G, AVR_INTERFACE_SELECTION);
//     std::printf("Interface Selection = %u\n", ifaceSel);

//     // 12) Clock & Reset Control
//     //     Pulse Design Reset, then read PLL lock status
//     ClkRstWriteReg(ePCIe, AVR_CLK_RST_CTRL_RESET_REGS, (1 << 9));
//     uint32_t pllLocked = ClkRstReadReg(ePCIe, AVR_CLK_RST_CTRL_PLL_LOCK_STATUS);
//     std::printf("PLL Locked = %u\n", pllLocked);

//     return 0;
// }

