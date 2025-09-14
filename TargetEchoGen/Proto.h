#ifndef PROTO_H
#define PROTO_H

//#define LIL_ENDIAN
#define BIG_ENDIAN
#define CC77_PACKET_STRUCT
#define SEND_MAGIC_BYTE_new     0xCC77
#define RECV_MAGIC_BYTE_new     0xAA88

typedef unsigned short int ushort;
typedef unsigned int uint;
typedef unsigned char uchar;

#define CMD_REG_WRITE       0x01
#define CMD_REG_READ        0x02
#define CMD_ACTION          0x03
#define CMD_GET_PROP        0x05

//#define SEND_MAGIC_BYTE 0xCCAABBEE
//#define RCEV_MAGIC_BYTE 0xCCBBDDFF

#define SEND_MAGIC_BYTE 0xCC77
#define RCEV_MAGIC_BYTE 0xAA88

#define CMD_BULK_WRITE 0x0001
#define CMD_BULK_READ 0x0002


#include <string.h>
#include <stdio.h>

class Proto
{
public:
    Proto();

    enum ACTION_TYPE
    {
        START_TX,
        STOP_TX,
        START_RX,
        STOP_RX,
        FPGA_RESET
    };

    enum SET_PROP
    {
        CYTO_TX_FREQ1,
        CYTO_TX_FREQ2,
        CYTO_TX_VOLT1,
        CYTO_TX_VOLT2,
        CYTO_LOOPBACK_MODE,
        CYTO_TX_DEBUG_MODE,
        CYTO_RX_BW
    };

    enum LOOPBACK_MODE
    {
        NO_LOOPBACK,
        SW_DDR_LOOPBACK,
        DMA_LOOPBACK,
        DAC_ADC_LOOPBACK,
        SW_LOOPBACK,
        TEST_MODE
    };

    //Register Write
    //return packet length
    ushort mPktRegWrite(uint a_nRegisterAddress, uint a_nRegisterData, char **a_byArrPacket);

    //Register Read
    //return packet length
    ushort mPktRegRead(uint a_nRegisterAddress, char **a_byArrPacket);

    //Bulk Write
    ushort mPktBulkWrite(uint a_nRegisterAddress, char *a_byBuff, uint a_nBuffLen, char** a_byArrPacket);

    ushort mPktBulkRead(uint a_nRegisterAddress, uint a_nPacketLen, char **a_byArrPacket);

    ushort mPktBulkReadUART(uint a_nRegisterAddress, uint a_nRegData, uint a_nPacketLen, char **a_byArrPacket);
    ushort mPktBulkWriteUART(uint a_nRegisterAddress, uint a_nRegData, uint a_nPacketLen, char *a_byArrPacket);

    int mPktParse(char *byArrPacket);
    uint mParseResponsePkt(const char byArrPktResp[16]);
    int mPktParseBulkRead(const char *byArrPktResp);
    ushort mGetPacketLength();
    uchar  mGetCommand();
    uint   mGetStatus();
    uint   mGetRegisterData();

    void mMemCpy(char *dst, uchar *src, size_t n);

    void mMemCpy(uchar *dst, char *src, size_t n);

    void mMemCpy(char *dst, char *src, size_t n);

    void mMemCpy(uchar *dst, uchar *src, size_t n);

    void mRevStr(uchar *src, size_t n);

    void mRevStr(char *src, size_t n);
    ushort  m_nPacketLength;
private:
    uint  m_nMagicBytes;
    uchar   m_chCommand;

    uint    m_nRegisterAddress;
    uint    m_nRegisterData;

    uint    m_nStatus;
};
#endif // PROTO_H
