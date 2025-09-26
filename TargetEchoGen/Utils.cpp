#include "Utils.h"
#include "AvrRegAddrDef.h"
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

static inline uint32_t AVRRegAccessRead(iface deviceType,AvrTegRegs::AvrTegOffset off) {
    uint32_t addr = AvrTegRegs::address(off);
    return readRegisterValue(deviceType, addr);
}

/// Write a full 32-bit LMK register
static inline int AVRRegAccessWrite(iface deviceType, AvrTegRegs::AvrTegOffset off, uint32_t propValue)
{
    uint32_t addr = AvrTegRegs::address(off);
    Utils::RegisterWrite(deviceType, addr, propValue);
    return 0;
}

int LMKRegAccessWrite(iface deviceType, AvrTegRegs::AvrTegOffset regOffset, unsigned int propValue)
{
    // --- debug print ---
    LOG_INFO("LMKRegAccessWrite(iface=%d, RegAddr=0x%X, Val=0x%X)",deviceType, regOffset, propValue);

    // Decode packed REG16_DEF: [31:16]=offset, [15:8]=bitPos, [7:0]=width
    uint32_t addr = address(regOffset);
    const unsigned int baseOffset = (addr >> 16) & 0xFFFF;
    const unsigned int bitPos     = (addr >> 8)  & 0xFF;
    const unsigned int width      = (addr) & 0xFF;

    // Turn numeric offset into our scoped enum
    auto off = static_cast<AvrTegRegs::AvrTegOffset>(baseOffset);

    // For sub-32-bit fields: read-mask-modify
    uint32_t regVal = 0;
    if (width < 32) {
        regVal    = AVRRegAccessRead(deviceType, off);
        uint32_t mask = ((1u << width) - 1u) << bitPos;
        regVal   &= ~mask;
        propValue &= (1u << width) - 1u;
    }
    // Insert new bits (or full-width overwrite)
    regVal |= static_cast<uint32_t>(propValue) << bitPos;

    // Write back
    AVRRegAccessWrite(deviceType, off, regVal);
    return 0;
}
//------------------------------------------------------------------------------
// Updated ProgramDefaultInLMKRegisters using the simplified LMKRegAccessWrite()
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Updated ProgramDefaultInLMKRegisters using the new three-arg API
//------------------------------------------------------------------------------

long ProgramDefaultInLMKRegisters(iface deviceType, unsigned int moduleID)
{
    unsigned int RegLmkFreq = 0;
    // Always apply the default 120 MHz LMK map
    switch(RegLmkFreq){
    case 1:
        LOG_INFO("**** DEFAULT SETTINGS FOR LMK @ 120 MHz ****");
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR1,  0x0000010);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR2,  0x0000010);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR3,  0x0000010);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR4,  0x0000010);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR5,  0x0000010);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR6,  0x0000010);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR7,  0x02808800);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR8,  0x00408800);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR9,  0x048E0210);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR10, 0x0888800);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR11, 0x48E0210);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR12, 0x1BF8880);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR13, 0x098600D);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR14, 0x1D81033);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR15, 0x0900000);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR16, 0x4000400);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR17, 0x00AA820);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR18, 0x00000006);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR19, 0x0080800);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR20, 0x047D18E0);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR21, 0x08000C1);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR22, 0x2EE0180);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR23, 0x0000EA6);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR24, 0x0000EA6);
    case 2:
        LOG_INFO("**** DEFAULT SETTINGS FOR LMK @ 120 MHz ****");
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR1,  0x0000010);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR2,  0x0000010);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR3,  0x0000010);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR4,  0x0000010);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR5,  0x0000010);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR6,  0x0000010);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR7,  0x02808800);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR8,  0x00408800);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR9,  0x048E0210);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR10, 0x0888800);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR11, 0x48E0210);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR12, 0x1BF8880);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR13, 0x098600D);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR14, 0x1D81033);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR15, 0x0900000);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR16, 0x4000400);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR17, 0x00AA820);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR18, 0x00000006);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR19, 0x0080800);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR20, 0x047D18E0);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR21, 0x08000C1);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR22, 0x2EE0180);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR23, 0x0000EA6);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR24, 0x0000EA6);
    case 3:
        LOG_INFO("**** DEFAULT SETTINGS FOR LMK @ 120 MHz ****");
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR1,  0x0000010);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR2,  0x0000010);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR3,  0x0000010);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR4,  0x0000010);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR5,  0x0000010);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR6,  0x0000010);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR7,  0x02808800);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR8,  0x00408800);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR9,  0x048E0210);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR10, 0x0888800);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR11, 0x48E0210);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR12, 0x1BF8880);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR13, 0x098600D);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR14, 0x1D81033);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR15, 0x0900000);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR16, 0x4000400);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR17, 0x00AA820);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR18, 0x00000006);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR19, 0x0080800);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR20, 0x047D18E0);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR21, 0x08000C1);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR22, 0x2EE0180);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR23, 0x0000EA6);
        LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR24, 0x0000EA6);
    default:
        LOG_INFO("Invalid value\n");
    }
    return 0;
}

void LMKDefaultSetting(iface deviceType)
{
    Utils::RegisterWrite(deviceType, 0x00005014, 0x00000000);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00000000);
    Utils::RegisterWrite(deviceType, 0x00001010, 0x00000201);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00000400);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00000401);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00000800);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00000801);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00000C00);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00000C01);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00001000);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00001001);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00001400);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00001401);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00001800);
    Utils::RegisterWrite(deviceType, 0x00001010, 0x50110001);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000180C);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000980C);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000980D);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00009C0C);
    Utils::RegisterWrite(deviceType, 0x00001010, 0x08110001);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000A00C);
    Utils::RegisterWrite(deviceType, 0x00001010, 0x91C04201);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000A00D);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000A80C);
    Utils::RegisterWrite(deviceType, 0x00001010, 0x91C04201);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000A80D);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000AC0C);
    Utils::RegisterWrite(deviceType, 0x00001010, 0x37F11001);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000AC0D);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000B00C);
    Utils::RegisterWrite(deviceType, 0x00001010, 0x130C01A1);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000B00D);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000B40C);
    Utils::RegisterWrite(deviceType, 0x00001010, 0x3B020661);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000B40D);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000B80C);
    Utils::RegisterWrite(deviceType, 0x00001010, 0x12000001);
}

void writeSpiSynth(iface deviceType, uint32_t data, uint32_t address, uint32_t numPostClock, bool syncEnAuto)
{
    uint32_t val;

    // 1) Set 5-bit ADDRESS field in reg 0x100C (bits [14:10])
    val = readRegisterValue(deviceType, 0x100C);
    // clear bits 10–14, then OR in (address & 0x1F) << 10
    val = (val & 0xFFFF83FFu)| ((address     & 0x1Fu) << 10);
    RegisterWrite(deviceType, 0x100C, val);

    // 2) Set 27-bit DATA field in reg 0x1010 (bits [31:5])
    val = readRegisterValue(deviceType, 0x1010);
    // clear bits 5–31, then OR in (data & 0x07FFFFFF) << 5
    val = (val & 0x0000001Fu)| ((data        & 0x07FFFFFFu) << 5);
    RegisterWrite(deviceType, 0x1010, val);

    // 3) Set NUM_POSTCLOCK in reg 0x100C (bits [9:2])
    val = readRegisterValue(deviceType, 0x100C);
    // clear bits 2–9, then OR in (numPostClock & 0xFF) << 2
    val = (val & 0xFFFFFC03u) | ((numPostClock & 0xFFu) << 2);
    RegisterWrite(deviceType, 0x100C, val);

    // 4) Set SYNC_EN_AUTO in reg 0x100C (bit 15)
    val = readRegisterValue(deviceType, 0x100C);
    // clear bit 15, then OR in (syncEnAuto & 1) << 15
    val = (val & 0xFFFF7FFFu) | (((uint32_t)syncEnAuto & 0x1u) << 15);
    RegisterWrite(deviceType, 0x100C, val);

    // 5) Toggle SPI-WRITE strobe (bit 0) in reg 0x100C
    val = readRegisterValue(deviceType, 0x100C);
    RegisterWrite(deviceType, 0x100C, val | 0x1u);     // set bit 0
    val = readRegisterValue(deviceType, 0x100C);
    RegisterWrite(deviceType, 0x100C, val & ~0x1u);    // clear bit 0

    // 6) Poll until write complete: status bit 0 in reg 0x1014 goes high
    do {
        val = readRegisterValue(deviceType, 0x1014);
    } while ((val & 0x1u) == 0);
}

}
