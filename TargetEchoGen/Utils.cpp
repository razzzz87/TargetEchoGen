#include "Utils.h"
#include "AvrRegAddrDef.h"
#include "log.h"
//#include "RegAccessWrappers.h"
namespace Utils
{

inline UartSerial* serial = nullptr;
inline EthernetSocket* eth1G = nullptr;
inline EthernetSocketPL1G* ethPl1G = nullptr;
inline EthernetSocket10G* eth10G = nullptr;
inline QDialog* progressDialog = nullptr;

uint32_t setBit(uint32_t& value, uint32_t pos)
{

    LOG_INFO("[Utils::setBit] Before: Val:0x%08X, Pos:%d",value,pos);
    value |= (1U << pos);
    LOG_INFO("[Utils::setBit] After: 0x%08X,",value);
    return value;
}

uint64_t setBit64(uint64_t& value, int pos) {

    LOG_INFO("[Utils::setBit] Before: Val:0x%08X, Pos:%d",value,pos);
    value |= (1U << pos);
    LOG_INFO("[Utils::setBit] After: 0x%08X,",value);
    return value;
}

uint32_t clearBit(uint32_t& value, int pos) {

    LOG_INFO("[Utils::clearBit] Before: Val:0x%08X, Pos:%d",value,pos);
    value &= ~(1U << pos);
    LOG_INFO("[Utils::clearBit] After: Val:0x%08X, Pos:%d",value,pos);
    return value;
}
uint64_t clearBit64(uint64_t& value, int pos) {

    LOG_INFO("[Utils::clearBit] Before: Val:0x%08X, Pos:%d",value,pos);
    value &= ~(1U << pos);
    LOG_INFO("[Utils::clearBit] After: Val:0x%08X, Pos:%d",value,pos);
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
inline const char* WriteErrorToString(WriteRegError err)
{
    switch (err) {
    case WriteRegError::SUCCESS:              return "SUCCESS - Register write completed successfully.";
    case WriteRegError::SERIAL_NULL:          return "SERIAL_NULL - Serial interface not initialized.";
    case WriteRegError::SERIAL_SEND_FAIL:     return "SERIAL_SEND_FAIL - Serial transmission failed.";
    case WriteRegError::ETHPL1G_NULL:         return "ETHPL1G_NULL - 1G Ethernet interface not initialized.";
    case WriteRegError::ETHPL1G_SEND_FAIL:    return "ETHPL1G_SEND_FAIL - Failed to send over 1G Ethernet.";
    case WriteRegError::ETH10G_NULL:          return "ETH10G_NULL - 10G Ethernet interface not initialized.";
    case WriteRegError::ETH10G_SEND_FAIL:     return "ETH10G_SEND_FAIL - Failed to send over 10G Ethernet.";
    case WriteRegError::ETH10G_RECV_FAIL:     return "ETH10G_RECV_FAIL - No response or timeout from 10G device.";
    case WriteRegError::INVALID_INTERFACE:    return "INVALID_INTERFACE - Unsupported interface for write.";
    default:                                  return "UNKNOWN_ERROR - Undefined write error code.";
    }
}

inline const char* ReadErrorToString(ReadRegError err)
{
    switch (err) {
    case ReadRegError::SUCCESS:               return "SUCCESS - Register read completed successfully.";
    case ReadRegError::SERIAL_NULL:           return "SERIAL_NULL - Serial interface not initialized.";
    case ReadRegError::SERIAL_SEND_FAIL:      return "SERIAL_SEND_FAIL - Serial send failed.";
    case ReadRegError::SERIAL_RECV_FAIL:      return "SERIAL_RECV_FAIL - Serial receive failed.";
    case ReadRegError::ETHPL1G_NULL:          return "ETHPL1G_NULL - 1G Ethernet interface not initialized.";
    case ReadRegError::ETHPL1G_RECV_FAIL:     return "ETHPL1G_RECV_FAIL - No response from 1G Ethernet device.";
    case ReadRegError::ETH10G_NULL:           return "ETH10G_NULL - 10G Ethernet interface not initialized.";
    case ReadRegError::ETH10G_SEND_FAIL:      return "ETH10G_SEND_FAIL - Failed to send over 10G Ethernet.";
    case ReadRegError::ETH10G_RECV_FAIL:      return "ETH10G_RECV_FAIL - Failed to receive from 10G Ethernet.";
    case ReadRegError::INVALID_INTERFACE:     return "INVALID_INTERFACE - Unsupported interface for read.";
    case ReadRegError::UNKNOWN_ERROR:         return "UNKNOWN_ERROR - Unexpected read error occurred.";
    default:                                  return "UNDEFINED_ERROR - Unknown read error code.";
    }
}

QString ifaceToQString(iface type)
{
    switch (type)
    {
    case eETHPS1G:   return "ETH PS1G";
    case eETHPL1G:   return "ETH PL1G";
    case eETH10G:    return "ETH 10G";
    case eSERIAL:    return "SERIAL";
    case ePLSERIAL:  return "PL SERIAL";
    case ePCIe:      return "PCIe";
    case eNONE:
    default:         return "NONE";
    }
}

const char* ifaceToCStr(iface type)
{
    switch (type)
    {
    case eETHPS1G:   return "ETH PS1G";
    case eETHPL1G:   return "ETH PL1G";
    case eETH10G:    return "ETH 10G";
    case eSERIAL:    return "SERIAL";
    case ePLSERIAL:  return "PL SERIAL";
    case ePCIe:      return "PCIe";
    case eNONE:
    default:         return "NONE";
    }
}

GuiReadRegError readRegisterValue(iface deviceType, QLineEdit* lineEditAddr, QLineEdit* lineEditVal)
{
    bool ok;
    uint addr = lineEditAddr->text().toUInt(&ok, 16);
    LOG_INFO("[readRegisterValue] ENTER | Addr: 0x%08X", addr);

    if (!ok) {
        LOG_ERROR("[readRegisterValue] INVALID_ADDR_FORMAT | Raw: %s", lineEditAddr->text().toStdString().c_str());
        return GuiReadRegError::INVALID_ADDR_FORMAT;
    }

    char* byArrPkt = nullptr;
    char ByteArr64BitPakt[64] = {0};
    Proto protocolobj;
    int reg_val = 0;
    GuiReadRegError status = GuiReadRegError::SUCCESS;

    int pktLen = protocolobj.mPktRegRead(addr, &byArrPkt);

    switch (deviceType)
    {
    case iface::eSERIAL:
        if (!serial) {
            LOG_ERROR("[readRegisterValue] SERIAL_NULL");
            status = GuiReadRegError::SERIAL_NULL;
            break;
        }
        if (!serial->sendData(byArrPkt, pktLen)) {
            LOG_ERROR("[readRegisterValue] SERIAL_SEND_FAIL");
            status = GuiReadRegError::SERIAL_SEND_FAIL;
            break;
        }
        if (!serial->receiveData(ByteArr64BitPakt, pktLen)) {
            LOG_ERROR("[readRegisterValue] SERIAL_RECV_FAIL");
            status = GuiReadRegError::SERIAL_RECV_FAIL;
            break;
        }
        reg_val = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
        break;

    case iface::eETHPL1G:
        ethPl1G = EthernetSocketPL1G::getInstance();
        if (!ethPl1G) {
            LOG_ERROR("[readRegisterValue] ETHPL1G_NULL");
            status = GuiReadRegError::ETHPL1G_NULL;
            break;
        }
        if (!ethPl1G->sendData(byArrPkt, pktLen)) {
            LOG_ERROR("[readRegisterValue] ETHPL1G_SEND_FAIL");
            status = GuiReadRegError::ETHPL1G_SEND_FAIL;
            break;
        }
        {
            int RecvByte;
            if (!ethPl1G->receiveData(ByteArr64BitPakt, pktLen, RecvByte)) {
                LOG_ERROR("[readRegisterValue] ETHPL1G_RECV_FAIL");
                status = GuiReadRegError::ETHPL1G_RECV_FAIL;
                break;
            }
            reg_val = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
        }
        break;

    case iface::eETH10G:
        if (!eth10G) {
            LOG_ERROR("[readRegisterValue] ETH10G_NULL");
            status = GuiReadRegError::ETH10G_NULL;
            break;
        }
        if (!eth10G->sendData(byArrPkt, pktLen, eth10G->RemoteIP.toStdString(), eth10G->Port)) {
            LOG_ERROR("[readRegisterValue] ETH10G_SEND_FAIL");
            status = GuiReadRegError::ETH10G_SEND_FAIL;
            break;
        }
        {
            std::string senderIp;
            uint16_t senderport;
            if (!eth10G->receiveData(ByteArr64BitPakt, pktLen, senderIp, senderport)) {
                LOG_ERROR("[readRegisterValue] ETH10G_RECV_FAIL");
                status = GuiReadRegError::ETH10G_RECV_FAIL;
                break;
            }
            reg_val = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
        }
        break;

    case iface::ePCIe:
        LOG_INFO("[readRegisterValue] PCIe interface not implemented.");
        status = GuiReadRegError::INVALID_INTERFACE;
        break;

    default:
        LOG_ERROR("[readRegisterValue] INVALID_INTERFACE | Type: %d", static_cast<int>(deviceType));
        status = GuiReadRegError::INVALID_INTERFACE;
        break;
    }

    delete[] byArrPkt;

    if (status == GuiReadRegError::SUCCESS) {
        lineEditVal->setText(QString("%1").arg(reg_val, 8, 16, QChar('0')).toUpper());
        LOG_INFO("[readRegisterValue] EXIT | Addr: 0x%08X | RegVal: 0x%08X", addr, reg_val);
    } else {
        LOG_INFO("[readRegisterValue] EXIT | Addr: 0x%08X | RegVal: <FAILED>", addr);
    }

    return status;
}

ReadResult readRegisterValue(iface deviceType, uint32_t addr)
{
    LOG_INFO("[readRegisterValue] ENTER | Addr: 0x%08X | Interface: %d", addr, static_cast<int>(deviceType));

    ReadResult result = {0, ReadRegError::UNKNOWN_ERROR};
    char* byArrPkt = nullptr;
    char ByteArr64BitPakt[64] = {0};
    Proto protocolobj;

    int pktLen = protocolobj.mPktRegRead(addr, &byArrPkt);

    switch (deviceType)
    {
    case iface::eSERIAL:
    {
        serial = UartSerial::getInstance();
        if (!serial) {
            LOG_ERROR("[readRegisterValue] SERIAL_NULL | Addr: 0x%08X", addr);
            result.status = ReadRegError::SERIAL_NULL;
            break;
        }
        if (!serial->sendData(byArrPkt, pktLen)) {
            LOG_ERROR("[readRegisterValue] SERIAL_SEND_FAIL | Addr: 0x%08X", addr);
            result.status = ReadRegError::SERIAL_SEND_FAIL;
            break;
        }
        if (serial->receiveData(ByteArr64BitPakt, pktLen)) {
            result.value = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
            result.status = ReadRegError::SUCCESS;
        } else {
            LOG_ERROR("[readRegisterValue] SERIAL_RECV_FAIL | Addr: 0x%08X", addr);
            result.status = ReadRegError::SERIAL_RECV_FAIL;
        }
        break;
    }
    case iface::eETHPL1G:
    {
        ethPl1G = EthernetSocketPL1G::getInstance();
        if (!ethPl1G) {
            LOG_ERROR("[readRegisterValue] ETHPL1G_NULL | Addr: 0x%08X", addr);
            result.status = ReadRegError::ETHPL1G_NULL;
            break;
        }
        if (ethPl1G->sendData(byArrPkt, pktLen)) {
            int RecvByte;
            if (ethPl1G->receiveData(ByteArr64BitPakt, pktLen, RecvByte)) {
                result.value = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
                result.status = ReadRegError::SUCCESS;
            } else {
                LOG_ERROR("[readRegisterValue] ETHPL1G_RECV_FAIL | Addr: 0x%08X", addr);
                result.status = ReadRegError::ETHPL1G_RECV_FAIL;
            }
        }
        break;
    }
    case iface::ePCIe:
    {
        LOG_INFO("[readRegisterValue] INVALID_INTERFACE (PCIe) | Addr: 0x%08X", addr);
        result.status = ReadRegError::INVALID_INTERFACE;
        break;
    }

    case iface::eETH10G:
    {
        eth10G = EthernetSocket10G::getInstance();
        if (!eth10G) {
            LOG_ERROR("[readRegisterValue] ETH10G_NULL | Addr: 0x%08X", addr);
            result.status = ReadRegError::ETH10G_NULL;
            break;
        }
        if (!eth10G->sendData(byArrPkt, pktLen, eth10G->RemoteIP.toStdString(), eth10G->Port)) {
            LOG_ERROR("[readRegisterValue] ETH10G_SEND_FAIL | Addr: 0x%08X", addr);
            result.status = ReadRegError::ETH10G_SEND_FAIL;
            break;
        }
        std::string senderIp;
        uint16_t senderport;
        if (eth10G->receiveData(ByteArr64BitPakt, pktLen, senderIp, senderport)) {
            result.value = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
            result.status = ReadRegError::SUCCESS;
        } else {
            LOG_ERROR("[readRegisterValue] ETH10G_RECV_FAIL | Addr: 0x%08X", addr);
            result.status = ReadRegError::ETH10G_RECV_FAIL;
        }
        break;
    }

    default:
        LOG_INFO("[readRegisterValue] INVALID_INTERFACE | Addr: 0x%08X", addr);
        result.status = ReadRegError::INVALID_INTERFACE;
        break;
    }

    delete[] byArrPkt;
    LOG_INFO("[readRegisterValue] EXIT | Addr: 0x%08X | RegVal: 0x%08X | Status: %d",addr,result.value, static_cast<int>(result.status));

    return result;
}

uint32_t RegRead(iface deviceType, uint uiAddr)
{
    LOG_INFO("[RegRead] ENTER | Addr: 0x%08X", uiAddr);

    ReadResult result = readRegisterValue(deviceType, uiAddr);
    if (result.status != ReadRegError::SUCCESS) {
        LOG_ERROR("[RegRead] FAILED | Addr: 0x%08X | Code: %d | Desc: %s",uiAddr,static_cast<int>(result.status),
                  ReadErrorToString(result.status));
    } else {
        LOG_INFO("[RegRead] SUCCESS | Addr: 0x%08X | Value: 0x%08X",uiAddr, result.value);
    }

    LOG_INFO("[RegRead] EXIT | Addr: 0x%08X", uiAddr);
    return result.value;
}

bool RegWrite(iface deviceType, uint iaddr, uint ival)
{
    LOG_INFO("[RegWrite] ENTER | Addr: 0x%08X | Val: 0x%08X", iaddr, ival);

    // Perform the low-level write
    WriteRegError status = RegisterWrite(deviceType, iaddr, ival);

    // Log the outcome with descriptive message
    if (status == WriteRegError::SUCCESS) {
        LOG_INFO("[RegWrite] SUCCESS | Addr: 0x%08X | Val: 0x%08X", iaddr, ival);
    } else {
        LOG_ERROR("[RegWrite] FAILED | Addr: 0x%08X | Val: 0x%08X | Code: %d | Desc: %s",
                  iaddr,
                  ival,
                  static_cast<int>(status),
                  WriteErrorToString(status));
    }

    // Final exit log showing summary
    LOG_INFO("[RegWrite] EXIT | Addr: 0x%08X | Result: %s",
             iaddr,
             (status == WriteRegError::SUCCESS) ? "SUCCESS" : "FAIL");

    return (status == WriteRegError::SUCCESS);
}

WriteRegError RegisterWrite(iface deviceType, uint iaddr, uint ival)
{
    LOG_INFO("[RegisterWrite] ENTER | Addr: 0x%08X | Val: 0x%08X", iaddr, ival);
    char* byArrPkt = nullptr;
    Proto protocolobj;
    int pktLen = protocolobj.mPktRegWrite(iaddr, ival, &byArrPkt);

    switch (deviceType)
    {
    case iface::eSERIAL:
    {
        serial = UartSerial::getInstance();
        if (!serial) {
            LOG_ERROR("[RegisterWrite] SERIAL_NULL");
            return WriteRegError::SERIAL_NULL;
        }
        if (!serial->sendData(byArrPkt, pktLen)) {
            LOG_ERROR("[RegisterWrite] SERIAL_SEND_FAIL");
            return WriteRegError::SERIAL_SEND_FAIL;
        }
        break;
    }
    case iface::eETHPL1G:
    {
        ethPl1G = EthernetSocketPL1G::getInstance();
        if (!ethPl1G) {
            LOG_ERROR("[RegisterWrite] ETHPL1G_NULL");
            return WriteRegError::ETHPL1G_NULL;
        }
        if (!ethPl1G->sendData(byArrPkt, pktLen)) {
            LOG_ERROR("[RegisterWrite] ETHPL1G_SEND_FAIL");
            return WriteRegError::ETHPL1G_SEND_FAIL;
        }
        char ByteArr64BitPakt[64] = {0};
        int RecvByte;
        if (ethPl1G->receiveData(ByteArr64BitPakt, pktLen, RecvByte)) {
            protocolobj.mParseResponsePkt(ByteArr64BitPakt);
        }
        break;
    }
    case iface::eETH10G:
    {
        eth10G = EthernetSocket10G::getInstance();
        if (!eth10G) {
            LOG_ERROR("[RegisterWrite] ETH10G_NULL");
            return WriteRegError::ETH10G_NULL;
        }
        if (!eth10G->sendData(byArrPkt, pktLen, eth10G->RemoteIP.toStdString(), eth10G->Port)) {
            LOG_ERROR("[RegisterWrite] ETH10G_SEND_FAIL");
            return WriteRegError::ETH10G_SEND_FAIL;
        }
        char ByteArr64BitPakt[64] = {0};
        std::string senderIp;
        uint16_t senderport;
        if (eth10G->receiveData(ByteArr64BitPakt, pktLen, senderIp, senderport)) {
            protocolobj.mParseResponsePkt(ByteArr64BitPakt);
        } else {
            LOG_ERROR("[RegisterWrite] ETH10G_RECV_FAIL");
            return WriteRegError::ETH10G_RECV_FAIL;
        }
        break;
    }
    case iface::ePCIe:
    case iface::eNONE:
    case iface::eETHPS1G:
    case iface::ePLSERIAL:
        LOG_ERROR("[RegisterWrite] INVALID_INTERFACE");
        return WriteRegError::INVALID_INTERFACE;
    }
    delete[] byArrPkt;
    LOG_INFO("[RegisterWrite] EXIT | Addr: 0x%08X | Val: 0x%08X", iaddr, ival);
    return WriteRegError::SUCCESS;
}

void SpiCtrlWriteReg(iface deviceType, uint32_t uiAddr, uint32_t v)
{
    WriteRegError status = RegisterWrite(deviceType, uiAddr, v);
    if (status != WriteRegError::SUCCESS) {
        LOG_ERROR("[SpiCtrlWriteReg] Failed | Addr: 0x%08X | Val: 0x%08X | Status: %d", uiAddr, v, static_cast<int>(status));
    } else {
        LOG_INFO("[SpiCtrlWriteReg] Success | Addr: 0x%08X | Val: 0x%08X", uiAddr, v);
    }
}

uint32_t SpiCtrlReadReg(iface deviceType, uint32_t uiAddr)
{
    ReadResult result = readRegisterValue(deviceType, uiAddr);

    if (result.status != ReadRegError::SUCCESS) {
        LOG_ERROR("[SpiCtrlReadReg] Failed | Addr: 0x%08X | Status: %d", uiAddr, static_cast<int>(result.status));
        // Optionally: return a sentinel value or handle error
    } else {
        LOG_INFO("[SpiCtrlReadReg] Success | Addr: 0x%08X | Value: 0x%08X", uiAddr, result.value);
    }

    return result.value;
}

void Ddr3TdgWriteReg(iface deviceType, uint32_t off, uint32_t v)
{
    uint32_t addr = AVR_DDR3_TDG_BASE_ADDR + off;
    WriteRegError status = RegisterWrite(deviceType, addr, v);
    if (status != WriteRegError::SUCCESS) {
        LOG_ERROR("[Ddr3TdgWriteReg] Failed | Addr: 0x%08X | Val: 0x%08X | Status: %d", addr, v, static_cast<int>(status));
    } else {
        LOG_INFO("[Ddr3TdgWriteReg] Success | Addr: 0x%08X | Val: 0x%08X", addr, v);
    }
}

void Ddr3RwWriteReg(iface deviceType, uint32_t off, uint32_t v)
{
    uint32_t addr = AVR_DDR3_RW_ADAPTER_BASE_ADDR + off;
    WriteRegError status = RegisterWrite(deviceType, addr, v);
    if (status != WriteRegError::SUCCESS) {
        LOG_ERROR("[Ddr3RwWriteReg] Failed | Addr: 0x%08X | Val: 0x%08X | Status: %d", addr, v, static_cast<int>(status));
    } else {
        LOG_INFO("[Ddr3RwWriteReg] Success | Addr: 0x%08X | Val: 0x%08X", addr, v);
    }
}

void DacWriteReg(iface deviceType, uint32_t uiAddr, uint32_t v)
{
    WriteRegError status = RegisterWrite(deviceType, uiAddr, v);
    if (status != WriteRegError::SUCCESS) {
        LOG_ERROR("[DacWriteReg] Failed | Addr: 0x%08X | Val: 0x%08X | Status: %d", uiAddr, v, static_cast<int>(status));
    } else {
        LOG_INFO("[DacWriteReg] Success | Addr: 0x%08X | Val: 0x%08X", uiAddr, v);
    }
}

void Dac0AdaWriteReg(iface deviceType, uint32_t off, uint32_t v)
{
    uint32_t addr = AVR_DAC0_DDR3_ADAPTER_BASE_ADDR + off;
    WriteRegError status = RegisterWrite(deviceType, addr, v);
    if (status != WriteRegError::SUCCESS) {
        LOG_ERROR("[Dac0AdaWriteReg] Failed | Addr: 0x%08X | Val: 0x%08X | Status: %d", addr, v, static_cast<int>(status));
    } else {
        LOG_INFO("[Dac0AdaWriteReg] Success | Addr: 0x%08X | Val: 0x%08X", addr, v);
    }
}

void Dac1AdaWriteReg(iface deviceType, uint32_t off, uint32_t v)
{
    uint32_t addr = AVR_DAC1_DDR3_ADAPTER_BASE_ADDR + off;
    WriteRegError status = RegisterWrite(deviceType, addr, v);
    if (status != WriteRegError::SUCCESS) {
        LOG_ERROR("[Dac1AdaWriteReg] Failed | Addr: 0x%08X | Val: 0x%08X | Status: %d", addr, v, static_cast<int>(status));
    } else {
        LOG_INFO("[Dac1AdaWriteReg] Success | Addr: 0x%08X | Val: 0x%08X", addr, v);
    }
}

void LvdsIfWriteReg(iface deviceType, uint32_t off, uint32_t v)
{
    uint32_t addr = AVR_LVDS_INTERFACE_BASE_ADDR + off;
    WriteRegError status = RegisterWrite(deviceType, addr, v);
    if (status != WriteRegError::SUCCESS) {
        LOG_ERROR("[LvdsIfWriteReg] Failed | Addr: 0x%08X | Val: 0x%08X | Status: %d", addr, v, static_cast<int>(status));
    } else {
        LOG_INFO("[LvdsIfWriteReg] Success | Addr: 0x%08X | Val: 0x%08X", addr, v);
    }
}

void SpiFlashWriteReg(iface deviceType, uint32_t off, uint32_t v)
{
    uint32_t addr = AVR_SPI_FLASH_BASE_ADDR + off;
    WriteRegError status = RegisterWrite(deviceType, addr, v);
    if (status != WriteRegError::SUCCESS) {
        LOG_ERROR("[SpiFlashWriteReg] Failed | Addr: 0x%08X | Val: 0x%08X | Status: %d", addr, v, static_cast<int>(status));
    } else {
        LOG_INFO("[SpiFlashWriteReg] Success | Addr: 0x%08X | Val: 0x%08X", addr, v);
    }
}

void I2cSlvWriteReg(iface deviceType, uint32_t off, uint32_t v)
{
    uint32_t addr = AVR_I2C_SLAVE_CTRL_BASE_ADDR + off;
    WriteRegError status = RegisterWrite(deviceType, addr, v);
    if (status != WriteRegError::SUCCESS) {
        LOG_ERROR("[I2cSlvWriteReg] Failed | Addr: 0x%08X | Val: 0x%08X | Status: %d", addr, v, static_cast<int>(status));
    } else {
        LOG_INFO("[I2cSlvWriteReg] Success | Addr: 0x%08X | Val: 0x%08X", addr, v);
    }
}

void IfCommonWriteReg(iface deviceType, uint32_t off, uint32_t v)
{
    uint32_t addr = AVR_INTERFACE_BASE_ADDR + off;
    WriteRegError status = RegisterWrite(deviceType, addr, v);
    if (status != WriteRegError::SUCCESS) {
        LOG_ERROR("[IfCommonWriteReg] Failed | Addr: 0x%08X | Val: 0x%08X | Status: %d", addr, v, static_cast<int>(status));
    } else {
        LOG_INFO("[IfCommonWriteReg] Success | Addr: 0x%08X | Val: 0x%08X", addr, v);
    }
}

void ClkRstWriteReg(iface deviceType, uint32_t off, uint32_t v)
{
    uint32_t addr = AVR_CLK_RST_CTRL_BASE_ADDR + off;
    WriteRegError status = RegisterWrite(deviceType, addr, v);
    if (status != WriteRegError::SUCCESS) {
        LOG_ERROR("[ClkRstWriteReg] Failed | Addr: 0x%08X | Val: 0x%08X | Status: %d", addr, v, static_cast<int>(status));
    } else {
        LOG_INFO("[ClkRstWriteReg] Success | Addr: 0x%08X | Val: 0x%08X", addr, v);
    }
}


uint32_t Ddr3TdgReadReg(iface deviceType, uint32_t off)
{
    uint32_t addr = AVR_DDR3_TDG_BASE_ADDR + off;
    ReadResult result = readRegisterValue(deviceType, addr);
    if (result.status != ReadRegError::SUCCESS) {
        LOG_ERROR("[Ddr3TdgReadReg] Failed | Addr: 0x%08X | Status: %d", addr, static_cast<int>(result.status));
    } else {
        LOG_INFO("[Ddr3TdgReadReg] Success | Addr: 0x%08X | Value: 0x%08X", addr, result.value);
    }
    return result.value;
}

uint32_t Ddr3RwReadReg(iface deviceType, uint32_t off)
{
    uint32_t addr = AVR_DDR3_RW_ADAPTER_BASE_ADDR + off;
    ReadResult result = readRegisterValue(deviceType, addr);
    if (result.status != ReadRegError::SUCCESS) {
        LOG_ERROR("[Ddr3RwReadReg] Failed | Addr: 0x%08X | Status: %d", addr, static_cast<int>(result.status));
    } else {
        LOG_INFO("[Ddr3RwReadReg] Success | Addr: 0x%08X | Value: 0x%08X", addr, result.value);
    }
    return result.value;
}

uint32_t DacReadReg(iface deviceType, uint32_t uiAddr) {

    ReadResult result = readRegisterValue(deviceType, uiAddr);
    if (result.status != ReadRegError::SUCCESS) {
        LOG_ERROR("[DacReadReg] Failed | Addr: 0x%08X | Status: %d", uiAddr, static_cast<int>(result.status));
    } else {
        LOG_INFO("[DacReadReg] Success | Addr: 0x%08X | Value: 0x%08X", uiAddr, result.value);
    }
    return result.value;
}

uint32_t Dac0AdaReadReg(iface deviceType, uint32_t off)
{
    uint32_t addr = AVR_DAC0_DDR3_ADAPTER_BASE_ADDR + off;
    ReadResult result = readRegisterValue(deviceType, addr);
    if (result.status != ReadRegError::SUCCESS) {
        LOG_ERROR("[Dac0AdaReadReg] Failed | Addr: 0x%08X | Status: %d", addr, static_cast<int>(result.status));
    } else {
        LOG_INFO("[Dac0AdaReadReg] Success | Addr: 0x%08X | Value: 0x%08X", addr, result.value);
    }
    return result.value;
}

uint32_t Dac1AdaReadReg(iface deviceType, uint32_t off)
{
    uint32_t addr = AVR_DAC1_DDR3_ADAPTER_BASE_ADDR + off;
    ReadResult result = readRegisterValue(deviceType, addr);
    if (result.status != ReadRegError::SUCCESS) {
        LOG_ERROR("[Dac1AdaReadReg] Failed | Addr: 0x%08X | Status: %d", addr, static_cast<int>(result.status));
    } else {
        LOG_INFO("[Dac1AdaReadReg] Success | Addr: 0x%08X | Value: 0x%08X", addr, result.value);
    }
    return result.value;
}

uint32_t LvdsIfReadReg(iface deviceType, uint32_t off)
{
    uint32_t addr = AVR_LVDS_INTERFACE_BASE_ADDR + off;
    ReadResult result = readRegisterValue(deviceType, addr);
    if (result.status != ReadRegError::SUCCESS) {
        LOG_ERROR("[LvdsIfReadReg] Failed | Addr: 0x%08X | Status: %d", addr, static_cast<int>(result.status));
    } else {
        LOG_INFO("[LvdsIfReadReg] Success | Addr: 0x%08X | Value: 0x%08X", addr, result.value);
    }
    return result.value;
}

uint32_t SpiFlashReadReg(iface deviceType, uint32_t off)
{
    uint32_t addr = AVR_SPI_FLASH_BASE_ADDR + off;
    ReadResult result = readRegisterValue(deviceType, addr);
    if (result.status != ReadRegError::SUCCESS) {
        LOG_ERROR("[SpiFlashReadReg] Failed | Addr: 0x%08X | Status: %d", addr, static_cast<int>(result.status));
    } else {
        LOG_INFO("[SpiFlashReadReg] Success | Addr: 0x%08X | Value: 0x%08X", addr, result.value);
    }
    return result.value;
}

uint32_t ManufReadReg(iface deviceType, uint32_t off)
{
    uint32_t addr = AVR_MANUF_CTRL_STATUS_BASE_ADDR + off;
    ReadResult result = readRegisterValue(deviceType, addr);
    if (result.status != ReadRegError::SUCCESS) {
        LOG_ERROR("[ManufReadReg] Failed | Addr: 0x%08X | Status: %d", addr, static_cast<int>(result.status));
    } else {
        LOG_INFO("[ManufReadReg] Success | Addr: 0x%08X | Value: 0x%08X", addr, result.value);
    }
    return result.value;
}

uint32_t I2cSlvReadReg(iface deviceType, uint32_t off)
{
    uint32_t addr = AVR_I2C_SLAVE_CTRL_BASE_ADDR + off;
    ReadResult result = readRegisterValue(deviceType, addr);
    if (result.status != ReadRegError::SUCCESS) {
        LOG_ERROR("[I2cSlvReadReg] Failed | Addr: 0x%08X | Status: %d", addr, static_cast<int>(result.status));
    } else {
        LOG_INFO("[I2cSlvReadReg] Success | Addr: 0x%08X | Value: 0x%08X", addr, result.value);
    }
    return result.value;
}

uint32_t IfCommonReadReg(iface deviceType, uint32_t off)
{
    uint32_t addr = AVR_INTERFACE_BASE_ADDR + off;
    ReadResult result = readRegisterValue(deviceType, addr);
    if (result.status != ReadRegError::SUCCESS) {
        LOG_ERROR("[IfCommonReadReg] Failed | Addr: 0x%08X | Status: %d", addr, static_cast<int>(result.status));
    } else {
        LOG_INFO("[IfCommonReadReg] Success | Addr: 0x%08X | Value: 0x%08X", addr, result.value);
    }
    return result.value;
}

uint32_t ClkRstReadReg(iface deviceType, uint32_t off)
{
    uint32_t addr = AVR_CLK_RST_CTRL_BASE_ADDR + off;
    ReadResult result = readRegisterValue(deviceType, addr);
    if (result.status != ReadRegError::SUCCESS) {
        LOG_ERROR("[ClkRstReadReg] Failed | Addr: 0x%08X | Status: %d", addr, static_cast<int>(result.status));
    } else {
        LOG_INFO("[ClkRstReadReg] Success | Addr: 0x%08X | Value: 0x%08X", addr, result.value);
    }
    return result.value;
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

    Utils::SpiCtrlWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR+0x3C, uiAddr);
    Utils::SpiCtrlWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR + 0x54, 0x0);
    Utils::SpiCtrlWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR + 0x38, 0x1);
    Utils::SpiCtrlWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR + 0x38, 0x0);
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
    LOG_INFO("[SpiDacRead] Addr: 0x%08X", Address);
    DacWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR+0x08, sel);      // Select DAC
    DacWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR+0x0C, Address);  // Set Address
    DacWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR+0x04, 0x1);      // Read Enable
    DacWriteReg(deviceType, AVR_SPI_CTRL_BASE_ADDR+0x04, 0x0);      // Read Disable
    uint32_t data = DacReadReg(deviceType, AVR_SPI_CTRL_BASE_ADDR+0x1C);  // Read Data
    LOG_INFO("[SpiDacRead] Value: 0x%08X", data);
    return data;
}




// Optional: central text lookup
QString statusToText(DeviceStatus st)
{
    switch (st) {
    case DeviceStatus::Active:       return "Active";
    case DeviceStatus::Inactive:     return "Inactive";
    case DeviceStatus::Connected:    return "Connected";
    case DeviceStatus::Disconnected: return "Disconnected";
    case DeviceStatus::Selected:     return "Selected";
    case DeviceStatus::Idle:         return "Idle";
    }
    return "Unknown";
}

// Optional: central icon lookup
QString statusToIcon(DeviceStatus st)
{
    switch (st) {
    case DeviceStatus::Disconnected: return ":/images/icons8-red-notconn-cross-48.png";
    case DeviceStatus::Selected:     return ":/images/green-checked-radio-button-48.png";
    case DeviceStatus::Active:       return ":/images/green-checked-radio-button-48.png";   // same as example
    case DeviceStatus::Inactive:     return ":/images/led-green_dim.png";
    case DeviceStatus::Idle:         return ":/images/led-green_dim.png";
    default:                         return ":/images/led-green_dim.png";
    }
}

}

