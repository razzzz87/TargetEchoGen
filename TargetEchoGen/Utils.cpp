#include "Utils.h"

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
        eth1G = EthernetSocket::getInstance();
        if (!eth1G) {
            LOG_TO_FILE("ERROR: Ethernet pointer is null.");
            return -1;
        }
        if(!eth1G->sendData(byArrPkt,pktLen,eth1G->RemoteIP.toStdString(),eth1G->Port)){
            LOG_TO_FILE("Sent filed!!!<eth1G>");
        }
        else
        {
            std::string senderIp;
            uint16_t senderport;
            if(eth1G->receiveData(ByteArr64BitPakt,pktLen,senderIp,senderport))
            {
                reg_val = protocolobj.mParseResponsePkt(ByteArr64BitPakt);
                LOG_TO_FILE("REG_VAL:0x%08X",reg_val);
            }else{
                LOG_TO_FILE("Receive filed!!!<eth1G>");
            }
        }
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
    return reg_val;
    LOG_INFO("[Utils::readRegisterValue] <EXIT> Addr:0x%08X",addr);
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
}
