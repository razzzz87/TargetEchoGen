#include "Utils.h"
#include "AvrRegAddrDef.h"
//#include "RegAccessWrappers.h"
namespace Utils
{

inline UartSerial* serial = nullptr;
inline EthernetSocket* eth1G = nullptr;
inline EthernetSocketPL1G* ethPl1G = nullptr;
inline EthernetSocket10G* eth10G = nullptr;
inline QDialog* progressDialog = nullptr;

uint32_t setBit(uint32_t& value, int pos)
{

    LOG_INFO("[BitUtils::setBit] Before: Val:0x08X, Pos:%d",value,pos);
    value |= (1U << pos);
    LOG_INFO("[BitUtils::setBit] After: 0x08X,",value);
    return value;
}

uint64_t setBit64(uint64_t& value, int pos) {

    LOG_INFO("[BitUtils::setBit] Before: Val:0x08X, Pos:%d",value,pos);
    value |= (1U << pos);
    LOG_INFO("[BitUtils::setBit] After: 0x08X,",value);
    return value;
}

uint32_t clearBit(uint32_t& value, int pos) {

    LOG_INFO("[BitUtils::clearBit] Before: Val:0x08X, Pos:%d",value,pos);
    value &= ~(1U << pos);
    LOG_INFO("[BitUtils::clearBit] After: Val:0x08X, Pos:%d",value,pos);
    return value;
}
uint64_t clearBit64(uint64_t& value, int pos) {

    LOG_INFO("[BitUtils::clearBit] Before: Val:0x08X, Pos:%d",value,pos);
    value &= ~(1U << pos);
    LOG_INFO("[BitUtils::clearBit] After: Val:0x08X, Pos:%d",value,pos);
    return value;
}

uint32_t setBits(uint32_t value, int start, int end)
{
    uint32_t mask = ((1U << (end - start + 1)) - 1) << start;
    return value | mask;
}

uint32_t clearBits(uint32_t value, int start, int end) {
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
inline void setControlBit(uint32_t& reg_val, ControlBit bit, BitState state)
{
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
    case ePLSERIAL:
        break;
    }
    LOG_INFO("[Utils::RegisterWrite] <EXIT>");
}

void SpiCtrlWriteReg(iface deviceType, uint32_t uiAddr, uint32_t v) {
    RegisterWrite(deviceType,uiAddr, v);
}
uint32_t SpiCtrlReadReg(iface deviceType, uint32_t uiAddr) {
    return readRegisterValue(deviceType, uiAddr);
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

void DacWriteReg(iface deviceType, uint32_t uiAddr, uint32_t v) {
    RegisterWrite(deviceType,uiAddr, v);
}
uint32_t DacReadReg(iface deviceType, uint32_t uiAddr) {
    return readRegisterValue(deviceType,AVR_SPI_CTRL_BASE_ADDR+uiAddr);
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
void WriteSpiSynth(iface deviceType, uint32_t address, uint32_t data)
{
    // write  0x3C  address
    Utils::SpiCtrlWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR+0x3C, address);

    // if address <= 5 write 0x54 0x0 else write 0x54 0x103
    if (address <= 5) {
        Utils::SpiCtrlWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR + 0x54, 0x0);
    } else {
        Utils::SpiCtrlWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR + 0x54, 0x103);
    }
    // write  0x40  data
    Utils::SpiCtrlWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR + 0x40, data);
    // write  0x34  0x1
    Utils::SpiCtrlWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR + 0x34, 0x1);
    // write  0x34  0x0
    Utils::SpiCtrlWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR + 0x34, 0x0);
}

uint32_t ReadSpiSynth(iface deviceType,uint32_t uiAddr)
{
    uint32_t iRegVal = 0;
    // write  0x3C  address
    Utils::SpiCtrlWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR+0x3C, uiAddr);
    // if address <= 5 write 0x54 0x0 else write 0x54 0x103
    Utils::SpiCtrlWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR + 0x54, 0x0);
    // write  0x34  0x1
    Utils::SpiCtrlWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR + 0x38, 0x1);
    // write  0x34  0x0
    Utils::SpiCtrlWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR + 0x38, 0x0);
    // read  0x50 -> outData (read data)
    iRegVal = Utils::SpiCtrlReadReg(deviceType, AVR_SPI_CTRL_BASE_ADDR + 0x50);
    return iRegVal;
}

// -------------------------------
// DAC - SPI Write Function
// -------------------------------
void SpiDacWrite(iface deviceType, uint32_t Address, uint32_t Data, uint32_t sel)
{
    DacWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR+0x08, sel);      // Select DAC
    DacWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR+0x0C, Address);  // Set Address
    DacWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR+0x10, Data);     // Set Data
    DacWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR+0x00, 0x1);      // Write Enable
    DacWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR+0x00, 0x0);      // Write Disable
}

// -------------------------------
// DAC - SPI Read Function
// -------------------------------
uint32_t SpiDacRead(iface deviceType, uint32_t Address, uint32_t sel)
{
    DacWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR+0x08, sel);      // Select DAC
    DacWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR+0x0C, Address);  // Set Address
    DacWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR+0x04, 0x1);      // Read Enable
    DacWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR+0x04, 0x0);      // Read Disable
    uint32_t data = DacReadReg(deviceType, 0x101C);  // Read Data
    return data;
}
}

