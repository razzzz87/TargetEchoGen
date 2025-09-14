#include "Proto.h"
#include "log.h"
Proto::Proto()
{
    m_nPacketLength     =   0;
    m_nRegisterAddress  =   0;
    m_nRegisterData     =   0;
    m_chCommand         =   '\0';
    m_nMagicBytes       =   0;
    m_nStatus           =   0;
}

ushort Proto::mPktBulkWrite(uint a_nRegisterAddress, char *a_byBuff, uint a_nBuffLen, char** a_byArrPacket)
{
#ifdef CC77_PACKET_STRUCT
    int nPacketInitialLen	=	12	+	a_nBuffLen;
    *a_byArrPacket	=	new char[nPacketInitialLen];

    m_nPacketLength =   0;
    char *byArrPacket;
    byArrPacket	=	*a_byArrPacket;

    memset(byArrPacket,'\0',nPacketInitialLen);

    ushort nMagicByte =   SEND_MAGIC_BYTE_new;
    uchar *pchMagicByte;
    pchMagicByte =  (uchar *) &nMagicByte;
    mMemCpy(byArrPacket,pchMagicByte,2);
    m_nPacketLength +=  2;

    //length bytes (2 bytes)
    ushort nLen	=	nPacketInitialLen;
    uchar *pchPktLen	=	(uchar *) &nLen;
    mMemCpy(byArrPacket	+	m_nPacketLength,pchPktLen,2);
    m_nPacketLength +=  2;

    //cmnd bytes (4 bytes)
    char chCommand[4];
    chCommand[0]	=	'\0';
    chCommand[1]	=	CMD_REG_WRITE; //0x1:RW, 0x2:RR, 0x3:Action
    chCommand[2]	=	'\0';
    chCommand[3]	=	'\0';

    memcpy(byArrPacket	+	m_nPacketLength, chCommand,4);

    m_nPacketLength +=  4;

    uchar *pchRegisterAddress;
    pchRegisterAddress  =   (uchar *)&a_nRegisterAddress;
    mMemCpy(byArrPacket	+	m_nPacketLength,pchRegisterAddress,4);
    m_nPacketLength +=  4;

    memcpy(byArrPacket	+	m_nPacketLength, a_byBuff, a_nBuffLen);

    m_nPacketLength	+=	a_nBuffLen;

    return m_nPacketLength;
#else
    ushort packetLen = 12 + a_nBuffLen;  // Sync (4) + Len (2) + Cmd (2) + Addr (4) + Payload
    *a_byArrPacket = new char[packetLen];
    char *byArrPacket = *a_byArrPacket;
    memset(byArrPacket, '\0', packetLen);

    m_nPacketLength = 0;

    // Sync (4 bytes)
    uint sync = SEND_MAGIC_BYTE;
    mMemCpy(byArrPacket, (uchar*)&sync, 4);
    m_nPacketLength += 4;

    // Command (2 bytes)
    ushort cmd = CMD_BULK_WRITE;
    mMemCpy(byArrPacket + m_nPacketLength, (uchar*)&cmd, 2);
    m_nPacketLength += 2;

    // Length (2 bytes)
    ushort nLen = packetLen;
    mMemCpy(byArrPacket + m_nPacketLength, (uchar*)&nLen, 2);
    m_nPacketLength += 2;

    // Register Address (4 bytes)
    mMemCpy(byArrPacket + m_nPacketLength, (uchar*)&a_nRegisterAddress, 4);
    m_nPacketLength += 4;

    // Payload
    memcpy(byArrPacket + m_nPacketLength, a_byBuff, a_nBuffLen);
    m_nPacketLength += a_nBuffLen;

    return m_nPacketLength;
#endif
}

ushort Proto::mPktRegWrite(uint a_nRegisterAddress, uint a_nRegisterData, char** a_byArrPacket)
{
#ifdef CC77_PACKET_STRUCT
    *a_byArrPacket	=	new char[16];

    m_nPacketLength =   0;
    char *byArrPacket;
    byArrPacket	=	*a_byArrPacket;

    memset(byArrPacket,'\0',16);

    ushort nMagicByte =   SEND_MAGIC_BYTE_new;
    uchar *pchMagicByte;
    pchMagicByte =  (uchar *) &nMagicByte;
    mMemCpy(byArrPacket,pchMagicByte,2);
    m_nPacketLength +=  2;

    //length bytes (2 bytes)


    ushort nLen	=	16;
    uchar *pchPktLen	=	(uchar *) &nLen;
    mMemCpy(byArrPacket	+	m_nPacketLength,pchPktLen,2);
    m_nPacketLength +=  2;

    //cmnd bytes (4 bytes)
    char chCommand[4];
    chCommand[0]	=	'\0';
    chCommand[1]	=	CMD_REG_WRITE; //0x1:RW, 0x2:RR, 0x3:Action
    chCommand[2]	=	'\0';
    chCommand[3]	=	'\0';

    memcpy(byArrPacket	+	m_nPacketLength, chCommand,4);

    m_nPacketLength +=  4;

    uchar *pchRegisterAddress;
    pchRegisterAddress  =   (uchar *)&a_nRegisterAddress;
    mMemCpy(byArrPacket	+	m_nPacketLength,pchRegisterAddress,4);
    m_nPacketLength +=  4;

    uchar *pchRegisterData;
    pchRegisterData  =   (uchar *)&a_nRegisterData;
    mMemCpy(byArrPacket	+	m_nPacketLength,pchRegisterData,4);
    m_nPacketLength +=  4;

    return m_nPacketLength;
#else
    const ushort PACKET_SIZE = 16;
    *a_byArrPacket = new char[PACKET_SIZE];

    m_nPacketLength = 0;
    char* byArrPacket = *a_byArrPacket;
    memset(byArrPacket, '\0', PACKET_SIZE);

    // Magic Sync Byte (4 bytes)
    uint nSync = SEND_MAGIC_BYTE;
    uchar* pchSync = (uchar*)&nSync;
    mMemCpy(byArrPacket, pchSync, 4);
    m_nPacketLength += 4;

    // Command (2 bytes)
    ushort cmd = CMD_REG_WRITE;
    uchar* pchCmd = (uchar*)&cmd;
    mMemCpy(byArrPacket + m_nPacketLength, pchCmd, 2);
    m_nPacketLength += 2;

    // Packet Length (2 bytes)
    ushort nLen = PACKET_SIZE;
    uchar* pchPktLen = (uchar*)&nLen;
    mMemCpy(byArrPacket + m_nPacketLength, pchPktLen, 2);
    m_nPacketLength += 2;

    // Register Address (4 bytes)
    uchar* pchRegisterAddress = (uchar*)&a_nRegisterAddress;
    mMemCpy(byArrPacket + m_nPacketLength, pchRegisterAddress, 4);
    m_nPacketLength += 4;

    // Register Data (4 bytes)
    uchar* pchRegisterData = (uchar*)&a_nRegisterData;
    mMemCpy(byArrPacket + m_nPacketLength, pchRegisterData, 4);
    m_nPacketLength += 4;

    return m_nPacketLength;
#endif
}

ushort Proto::mPktBulkRead(uint a_nRegisterAddress, uint a_nPacketLen, char **a_byArrPacket)
{
#ifdef CC77_PACKET_STRUCT
    a_nPacketLen += 12;
    *a_byArrPacket	=	new char[16];

    m_nPacketLength =   0;
    char *byArrPacket;
    byArrPacket	=	*a_byArrPacket;

    memset(byArrPacket,'\0',16);

    ushort nMagicByte =   SEND_MAGIC_BYTE_new;
    uchar *pchMagicByte;
    pchMagicByte =  (uchar *) &nMagicByte;
    mMemCpy(byArrPacket,pchMagicByte,2);
    m_nPacketLength +=  2;

    //length bytes (2 bytes)
    ushort nLen	=	(ushort) a_nPacketLen;
    uchar *pchPktLen	=	(uchar *) &nLen;
    mMemCpy(byArrPacket	+	m_nPacketLength,pchPktLen,2);
    m_nPacketLength +=  2;

    //cmnd bytes (4 bytes)
    char chCommand[4];
    chCommand[0]	=	'\0';
    chCommand[1]	=	CMD_REG_READ; //0x1:RW, 0x2:RR, 0x3:Action
    chCommand[2]	=	'\0';
    chCommand[3]	=	'\0';

    memcpy(byArrPacket	+	m_nPacketLength, chCommand,4);
    m_nPacketLength +=  4;

    uchar *pchRegisterAddress;
    pchRegisterAddress  =   (uchar *)&a_nRegisterAddress;
    mMemCpy(byArrPacket	+	m_nPacketLength,pchRegisterAddress,4);
    m_nPacketLength +=  4;

    memset(byArrPacket	+	m_nPacketLength,'\0',4);
    m_nPacketLength +=  4;

    return m_nPacketLength;
#else
    ushort packetLen = static_cast<ushort>(a_nPacketLen);
    *a_byArrPacket = new char[packetLen];
    char *byArrPacket = *a_byArrPacket;
    memset(byArrPacket, '\0', packetLen);

    m_nPacketLength = 0;

    //Sync Byte (4 bytes)
    uint sync = SEND_MAGIC_BYTE;
    mMemCpy(byArrPacket, (uchar*)&sync, 4);
    m_nPacketLength += 4;

    //Command (2 bytes)
    //ushort cmd = CMD_BULK_READ;
    ushort cmd = 0x1102;
    mMemCpy(byArrPacket + m_nPacketLength, (uchar*)&cmd, 2);
    m_nPacketLength += 2;

    //Length (2 bytes)
    mMemCpy(byArrPacket + m_nPacketLength, (uchar*)&packetLen, 2);
    m_nPacketLength += 2;

    //Address (4 bytes)
    mMemCpy(byArrPacket + m_nPacketLength, (uchar*)&a_nRegisterAddress, 4);
    m_nPacketLength += 4;

    //Placeholder or padding (remaining)
    ushort remaining = packetLen - m_nPacketLength;
    if (remaining > 0) {
        memset(byArrPacket + m_nPacketLength, '\0', remaining);
        m_nPacketLength += remaining;
    }
    return m_nPacketLength;
#endif
}

ushort Proto::mPktRegRead(uint a_nRegisterAddress, char **a_byArrPacket)
{
#ifdef CC77_PACKET_STRUCT
    *a_byArrPacket	=	new char[16];

    m_nPacketLength =   0;
    char *byArrPacket;
    byArrPacket	=	*a_byArrPacket;

    memset(byArrPacket,'\0',16);

    ushort nMagicByte =   SEND_MAGIC_BYTE_new;
    uchar *pchMagicByte;
    pchMagicByte =  (uchar *) &nMagicByte;
    mMemCpy(byArrPacket,pchMagicByte,2);
    m_nPacketLength +=  2;

    //length bytes (2 bytes)
    ushort nLen	=	16;
    uchar *pchPktLen	=	(uchar *) &nLen;
    mMemCpy(byArrPacket	+	m_nPacketLength,pchPktLen,2);
    m_nPacketLength +=  2;

    //cmnd bytes (4 bytes)
    char chCommand[4];
    chCommand[0]	=	'\0';
    chCommand[1]	=	CMD_REG_READ; //0x1:RW, 0x2:RR, 0x3:Action
    chCommand[2]	=	'\0';
    chCommand[3]	=	'\0';

    memcpy(byArrPacket	+	m_nPacketLength, chCommand,4);
    m_nPacketLength +=  4;

    uchar *pchRegisterAddress;
    pchRegisterAddress  =   (uchar *)&a_nRegisterAddress;
    mMemCpy(byArrPacket	+	m_nPacketLength,pchRegisterAddress,4);
    m_nPacketLength +=  4;

    memset(byArrPacket	+	m_nPacketLength,'\0',4);
    m_nPacketLength +=  4;

    return m_nPacketLength;
#else
    const int PACKET_SIZE = 16;
    *a_byArrPacket = new char[PACKET_SIZE];

    m_nPacketLength = 0;
    char *byArrPacket = *a_byArrPacket;
    memset(byArrPacket, '\0', PACKET_SIZE);

    // Sync/magic bytes (4 bytes): CCAABBEE
    uint nSync = SEND_MAGIC_BYTE;
    uchar *pchSync = (uchar *)&nSync;
    mMemCpy(byArrPacket, pchSync, 4);
    m_nPacketLength += 4;

    // Command (2 bytes)
    ushort cmd = CMD_REG_READ;
    uchar *pchCmd = (uchar *)&cmd;
    mMemCpy(byArrPacket + m_nPacketLength, pchCmd, 2);
    m_nPacketLength += 2;

    // Length (2 bytes)
    ushort nLen = PACKET_SIZE;
    uchar *pchPktLen = (uchar *)&nLen;
    mMemCpy(byArrPacket + m_nPacketLength, pchPktLen, 2);
    m_nPacketLength += 2;

    // Register Address (4 bytes)
    uchar *pchRegAddr = (uchar *)&a_nRegisterAddress;
    mMemCpy(byArrPacket + m_nPacketLength, pchRegAddr, 4);
    m_nPacketLength += 4;

    // Padding (4 bytes)
    memset(byArrPacket + m_nPacketLength, '\0', 4);
    m_nPacketLength += 4;

    return m_nPacketLength;
#endif
}
#if 0
ushort Proto::mPktBulkReadUART(uint a_nRegisterAddress, uint a_nRegData, uint a_nPacketLen, char **a_byArrPacket)
{
    *a_byArrPacket	=	new char[16];

    m_nPacketLength =   0;
    char *byArrPacket;
    byArrPacket	=	*a_byArrPacket;

    memset(byArrPacket,'\0',16);

    ushort nMagicByte =   SEND_MAGIC_BYTE;
    uchar *pchMagicByte;
    pchMagicByte =  (uchar *) &nMagicByte;
    mMemCpy(byArrPacket,pchMagicByte,2);
    m_nPacketLength +=  2;

    //length bytes (2 bytes)
    ushort nLen	=	(ushort) a_nPacketLen;
    uchar *pchPktLen	=	(uchar *) &nLen;
    mMemCpy(byArrPacket	+	m_nPacketLength,pchPktLen,2);
    m_nPacketLength +=  2;

    //cmnd bytes (4 bytes)
    char chCommand[4];
    chCommand[0]	=	'\0';
    chCommand[1]	=	CMD_REG_READ; //0x1:RW, 0x2:RR, 0x3:Action
    chCommand[2]	=	'\0';
    chCommand[3]	=	'\0';

    memcpy(byArrPacket	+	m_nPacketLength, chCommand,4);
    m_nPacketLength +=  4;

    uchar *pchRegisterAddress;
    pchRegisterAddress  =   (uchar *)&a_nRegisterAddress;
    mMemCpy(byArrPacket	+	m_nPacketLength,pchRegisterAddress,4);
    m_nPacketLength +=  4;

    uchar *pchRegisterData;
    pchRegisterData  =   (uchar *)&a_nRegData;
    mMemCpy(byArrPacket	+	m_nPacketLength,pchRegisterData,4);
    m_nPacketLength +=  4;

    return m_nPacketLength;
}

ushort Proto::mPktBulkWriteUART(uint a_nRegisterAddress, uint a_nRegData, uint a_nPacketLen, char *byArrPacket)
{
    m_nPacketLength =   0;
    memset(byArrPacket,'\0',16);

    ushort nMagicByte =   SEND_MAGIC_BYTE;
    uchar *pchMagicByte;
    pchMagicByte =  (uchar *) &nMagicByte;
    mMemCpy(byArrPacket,pchMagicByte,2);
    m_nPacketLength +=  2;

    //length bytes (2 bytes)
    ushort nLen	=	(ushort) a_nPacketLen;
    uchar *pchPktLen	=	(uchar *) &nLen;
    mMemCpy(byArrPacket	+	m_nPacketLength,pchPktLen,2);
    m_nPacketLength +=  2;

    //cmnd bytes (4 bytes)
    char chCommand[4];
    chCommand[0]	=	'\0';
    chCommand[1]	=	CMD_REG_WRITE; //0x1:RW, 0x2:RR, 0x3:Action
    chCommand[2]	=	'\0';
    chCommand[3]	=	'\0';

    memcpy(byArrPacket	+	m_nPacketLength, chCommand,4);
    m_nPacketLength +=  4;

    uchar *pchRegisterAddress;
    pchRegisterAddress  =   (uchar *)&a_nRegisterAddress;
    mMemCpy(byArrPacket	+	m_nPacketLength,pchRegisterAddress,4);
    m_nPacketLength +=  4;

    uchar *pchRegisterData;
    pchRegisterData  =   (uchar *)&a_nRegData;
    mMemCpy(byArrPacket	+	m_nPacketLength,pchRegisterData,4);
    m_nPacketLength +=  4;

    return m_nPacketLength;
}
#endif
int Proto::mPktParse(char *a_byArrData)
{
    qDebug()<<"PArse packet";
    if(a_byArrData  ==  NULL)
        return -5;

    m_nPacketLength             =   0;
    int nParser                 =   -1;
    uchar chArrMagicBytes[2]    =   {'\0'};
    nParser++;
    chArrMagicBytes[0]          =   a_byArrData[nParser];
    nParser++;
    chArrMagicBytes[1]          =   a_byArrData[nParser];

#ifdef BIG_ENDIAN
    mRevStr(chArrMagicBytes, 2);
#endif

    m_nMagicBytes               =   *((uint *)chArrMagicBytes);

    if((m_nMagicBytes    ==  SEND_MAGIC_BYTE) |   (m_nMagicBytes   ==  RCEV_MAGIC_BYTE))
    {
        LOG_TO_FILE("Parse packet2");

        uchar chArrLength[2]    =   {'\0'};
        nParser++;
        chArrLength[0]    =   a_byArrData[nParser];
        nParser++;
        chArrLength[1]    =   a_byArrData[nParser];

#ifdef BIG_ENDIAN
        mRevStr(chArrLength,2);
#endif

        m_nPacketLength         =   *((uint *)chArrLength);

        if(m_nPacketLength  !=  16)
            return -2;

        nParser++;  //NULL Byte

        nParser++;
        m_chCommand =   a_byArrData[nParser];

        nParser+=2;

        uchar chArrStatus[4]    =   {'\0'};
        nParser++;
        chArrStatus[0]          =   a_byArrData[nParser];
        nParser++;
        chArrStatus[1]          =   a_byArrData[nParser];
        nParser++;
        chArrStatus[2]          =   a_byArrData[nParser];
        nParser++;
        chArrStatus[3]          =   a_byArrData[nParser];

#ifdef BIG_ENDIAN
        mRevStr(chArrStatus, 4);
#endif

        m_nStatus               =   *((uint *)chArrStatus);
        qDebug()<<"Parse packet3" << m_nStatus << *((uint *)chArrStatus);


        uchar chArrValue[4]     =   {'\0'};
        nParser++;
        chArrValue[0]           =   a_byArrData[nParser];
        nParser++;
        chArrValue[1]           =   a_byArrData[nParser];
        nParser++;
        chArrValue[2]           =   a_byArrData[nParser];
        nParser++;
        chArrValue[3]           =   a_byArrData[nParser];

#ifdef BIG_ENDIAN
        mRevStr(chArrValue, 4);
#endif

        m_nRegisterData         =   *((uint *)chArrValue);

    }
    else
    {
        qDebug()<<"Wrong Magic byte";
        return -1;  // wrong magic byte
    }
    return 0; //successful
}

uint Proto::mParseResponsePkt(const char byArrPktResp[16]) {

#ifdef CC77_PACKET_STRUCT
    size_t nParser = 0;

    // Extract magic byte (2 bytes, big endian)
    uint16_t m_nMagicBytes = (static_cast<uint8_t>(byArrPktResp[nParser]) << 8) |
                             static_cast<uint8_t>(byArrPktResp[nParser + 1]);
    nParser += 2;

    if (m_nMagicBytes != RECV_MAGIC_BYTE_new) { // 0xAA88
        LOG_TO_FILE("Wrong Magic byte");
        return 0;
    }

    // Extract packet length (2 bytes, big endian)
    uint16_t m_nPacketLength = (static_cast<uint8_t>(byArrPktResp[nParser]) << 8) |
                               static_cast<uint8_t>(byArrPktResp[nParser + 1]);
    nParser += 2;

    if (m_nPacketLength != 16) {
        LOG_TO_FILE("Invalid packet length");
        return 0;
    }

    // Extract command (4 bytes, big endian)
    uint32_t m_nCommand = (static_cast<uint8_t>(byArrPktResp[nParser]) << 24) |
                          (static_cast<uint8_t>(byArrPktResp[nParser + 1]) << 16) |
                          (static_cast<uint8_t>(byArrPktResp[nParser + 2]) << 8) |
                          static_cast<uint8_t>(byArrPktResp[nParser + 3]);
    nParser += 4;

    // Extract status (4 bytes, big endian)
    uint32_t m_nStatus = (static_cast<uint8_t>(byArrPktResp[nParser]) << 24) |
                         (static_cast<uint8_t>(byArrPktResp[nParser + 1]) << 16) |
                         (static_cast<uint8_t>(byArrPktResp[nParser + 2]) << 8) |
                         static_cast<uint8_t>(byArrPktResp[nParser + 3]);
    nParser += 4;

    // Extract register value (last 4 bytes)
    uint32_t m_nRegisterData = (static_cast<uint8_t>(byArrPktResp[nParser]) << 24) |
                               (static_cast<uint8_t>(byArrPktResp[nParser + 1]) << 16) |
                               (static_cast<uint8_t>(byArrPktResp[nParser + 2]) << 8) |
                               static_cast<uint8_t>(byArrPktResp[nParser + 3]);

    return m_nRegisterData;
#else

    size_t nParser = 0;
    // Extract magic bytes (4 bytes, big endian)
    uint m_nMagicBytes = (static_cast<uint8_t>(byArrPktResp[nParser]) << 24) |
                         (static_cast<uint8_t>(byArrPktResp[nParser + 1]) << 16) |
                         (static_cast<uint8_t>(byArrPktResp[nParser + 2]) << 8) |
                         static_cast<uint8_t>(byArrPktResp[nParser + 3]);
    nParser += 4;

    if (m_nMagicBytes != RCEV_MAGIC_BYTE) {
        LOG_TO_FILE("Wrong Magic byte");
        return 0;
    }

    // Extract command (2 bytes, if applicable)
    uint16_t m_nCommand = (static_cast<uint8_t>(byArrPktResp[nParser]) << 8) |
                          static_cast<uint8_t>(byArrPktResp[nParser + 1]);
    nParser += 2;

    // Extract packet length (2 bytes, big endian)
    uint16_t m_nPacketLength = (static_cast<uint8_t>(byArrPktResp[nParser]) << 8) |
                               static_cast<uint8_t>(byArrPktResp[nParser + 1]);
    nParser += 2;

    if (m_nPacketLength != 16) return 0;

    // Extract status (4 bytes, big endian)
    uint32_t m_nStatus = (static_cast<uint8_t>(byArrPktResp[nParser]) << 24) |
                         (static_cast<uint8_t>(byArrPktResp[nParser + 1]) << 16) |
                         (static_cast<uint8_t>(byArrPktResp[nParser + 2]) << 8) |
                         static_cast<uint8_t>(byArrPktResp[nParser + 3]);
    nParser += 4;

    // Extract register data (last 4 bytes)
    uint32_t m_nRegisterData = (static_cast<uint8_t>(byArrPktResp[nParser]) << 24) |
                               (static_cast<uint8_t>(byArrPktResp[nParser + 1]) << 16) |
                               (static_cast<uint8_t>(byArrPktResp[nParser + 2]) << 8) |
                               static_cast<uint8_t>(byArrPktResp[nParser + 3]);

    return m_nRegisterData;
#endif
}

int Proto::mPktParseBulkRead(const char* a_byArrData)
{

#ifdef CC77_PACKET_STRUCT
    size_t nParser = 0;

    // Extract magic byte (2 bytes, big endian)
    uint16_t m_nMagicBytes = (static_cast<uint8_t>(a_byArrData[nParser]) << 8) |
                             static_cast<uint8_t>(a_byArrData[nParser + 1]);
    nParser += 2;

    if (m_nMagicBytes != RECV_MAGIC_BYTE_new) { // 0xAA88
        LOG_TO_FILE("Wrong Magic byte");
        return 0;
    }

    // Extract packet length (2 bytes, big endian)
    uint16_t m_nPacketLength = (static_cast<uint8_t>(a_byArrData[nParser]) << 8) |
                               static_cast<uint8_t>(a_byArrData[nParser + 1]);
    nParser += 2;
    //m_nPacketLength =
    //if (m_nPacketLength != 16) {
    //    LOG_TO_FILE("Invalid packet length");
    //    return 0;
    //}

    // Extract command (4 bytes, big endian)
    uint32_t m_nCommand = (static_cast<uint8_t>(a_byArrData[nParser]) << 24) |
                          (static_cast<uint8_t>(a_byArrData[nParser + 1]) << 16) |
                          (static_cast<uint8_t>(a_byArrData[nParser + 2]) << 8) |
                          static_cast<uint8_t>(a_byArrData[nParser + 3]);
    nParser += 4;

    // Extract status (4 bytes, big endian)
    uint32_t m_nStatus = (static_cast<uint8_t>(a_byArrData[nParser]) << 24) |
                         (static_cast<uint8_t>(a_byArrData[nParser + 1]) << 16) |
                         (static_cast<uint8_t>(a_byArrData[nParser + 2]) << 8) |
                         static_cast<uint8_t>(a_byArrData[nParser + 3]);
    nParser += 4;

    // // Extract register value (last 4 bytes)
    // uint32_t m_nRegisterData = (static_cast<uint8_t>(a_byArrData[nParser]) << 24) |
    //                            (static_cast<uint8_t>(a_byArrData[nParser + 1]) << 16) |
    //                            (static_cast<uint8_t>(a_byArrData[nParser + 2]) << 8) |
    //                            static_cast<uint8_t>(a_byArrData[nParser + 3]);

    return m_nPacketLength;
#if 0
    if(a_byArrData  ==  NULL)
        return -5;
    //qDebug()<<"Packet parse step 1";
    m_nPacketLength             =   0;
    int nParser                 =   -1;
    uchar chArrMagicBytes[2]    =   {'\0'};
    nParser++;
    chArrMagicBytes[0]          =   a_byArrData[nParser];
    nParser++;
    chArrMagicBytes[1]          =   a_byArrData[nParser];

#ifdef BIG_ENDIAN
    mRevStr(chArrMagicBytes, 2);
#endif

    m_nMagicBytes               =   *((uint *)chArrMagicBytes);
    //qDebug("Magic Byte %x",m_nMagicBytes);
    LOG_INFO("SYNC MATCH0x%X",m_nMagicBytes);
    if((m_nMagicBytes    ==  SEND_MAGIC_BYTE_new) ||   (m_nMagicBytes   ==  RECV_MAGIC_BYTE_new))
    {

        uchar chArrLength[2]    =   {'\0'};
        nParser++;
        chArrLength[0]    =   a_byArrData[nParser];
        nParser++;
        chArrLength[1]    =   a_byArrData[nParser];

#ifdef BIG_ENDIAN
        mRevStr(chArrLength,2);
#endif

        m_nPacketLength         =   *((uint *)chArrLength);

        //TBD: Lenght could be any size
        //if(m_nPacketLength  !=  16)
        //    return -2;

        nParser++;  //NULL Byte

        nParser++;
        m_chCommand =   a_byArrData[nParser];

        nParser+=2;

        uchar chArrStatus[4]    =   {'\0'};
        nParser++;
        chArrStatus[0]          =   a_byArrData[nParser];
        nParser++;
        chArrStatus[1]          =   a_byArrData[nParser];
        nParser++;
        chArrStatus[2]          =   a_byArrData[nParser];
        nParser++;
        chArrStatus[3]          =   a_byArrData[nParser];

#ifdef BIG_ENDIAN
        mRevStr(chArrStatus, 4);
#endif

        m_nStatus               =   *((uint *)chArrStatus);

        uchar chArrValue[4]     =   {'\0'};
        nParser++;
        chArrValue[0]           =   a_byArrData[nParser];
        nParser++;
        chArrValue[1]           =   a_byArrData[nParser];
        nParser++;
        chArrValue[2]           =   a_byArrData[nParser];
        nParser++;
        chArrValue[3]           =   a_byArrData[nParser];

#ifdef BIG_ENDIAN
        mRevStr(chArrValue, 4);
#endif

        m_nRegisterData         =   *((uint *)chArrValue);

    }
    else
    {
        return -1;  // wrong magic byte
    }

    return 0;
#endif
#else
    if (!a_byArrData) {
        LOG_TO_FILE("Input data pointer is NULL");
        return -5;
    }

    const uint8_t* data = reinterpret_cast<const uint8_t*>(a_byArrData);
    int offset = 0;
    //Magic bytes (Big-endian decode)
    m_nMagicBytes = (unsigned int)((uint32_t(data[0]) << 24) | (uint32_t(data[1]) << 16) | (uint32_t(data[2]) << 8)  | uint32_t(data[3]));
    offset += 4;

    if (m_nMagicBytes != RCEV_MAGIC_BYTE) {
        LOG_TO_FILE("Invalid Magic Byte: 0x%08X", m_nMagicBytes);
        return -1;
    }

    // ✅ Command (1 byte)
    m_chCommand = data[offset];
    offset += 1;

    // Reserved byte (skip)
    offset += 1;

    //Length (2 bytes, Big-endian)
    m_nPacketLength = (uint16_t(data[offset]) << 8) | uint16_t(data[offset + 1]);
    offset += 2;

    //Status (4 bytes)
    m_nStatus = (uint32_t(data[offset]) << 24) |
                (uint32_t(data[offset + 1]) << 16) |
                (uint32_t(data[offset + 2]) << 8) |
                uint32_t(data[offset + 3]);
    offset += 4;
    return 0;
#endif

}

void Proto::mRevStr(char *src, size_t n)
{
    char *temp	=	new char[n];

    for(size_t i	=	0;	i	<	n; i++)
    {
        temp[i]	=	src[n	-	i	-	1];
    }

    delete src;
    src	=	temp;
    // qDebug("mRevStr %x",src);
}

void Proto::mRevStr(uchar *src, size_t n)
{
    uchar *temp	=	new uchar[n];

    for(size_t i	=	0;	i	<	n; i++)
    {
        temp[i]	=	src[n	-	i	-	1];
    }

    memcpy(src,temp,n);
    //delete src;
    delete temp;
}

void Proto::mMemCpy(char *dst,	uchar *src, size_t n)
{
#ifdef LIL_ENDIAN
    memcpy(dst,src,n);
#else
    for(size_t i	=	0;	i	<	n;	i++)
    {
        dst[n	-	i	-	1]	=	src[i];
    }
#endif
}

void Proto::mMemCpy(char *dst,	char *src, size_t n)
{
#ifdef LIL_ENDIAN
    memcpy(dst,src,n);
#else
    for(size_t i	=	0;	i	<	n;	i++)
    {
        dst[n	-	i	-	1]	=	src[i];
    }
#endif
}

void Proto::mMemCpy(uchar *dst,	uchar *src, size_t n)
{
#ifdef LIL_ENDIAN
    memcpy(dst,src,n);
#else
    for(size_t i	=	0;	i	<	n;	i++)
    {
        dst[n	-	i	-	1]	=	src[i];
    }
#endif
}

void Proto::mMemCpy(uchar *dst,	char *src, size_t n)
{
#ifdef LIL_ENDIAN
    memcpy(dst,src,n);
#else
    for(size_t i	=	0;	i	<	n;	i++)
    {
        dst[n	-	i	-	1]	=	src[i];
    }
#endif
}


ushort Proto::mGetPacketLength()
{
    return m_nPacketLength;
}

uchar Proto::mGetCommand()
{
    return m_chCommand;
}

uint Proto::mGetStatus()
{
    return m_nStatus;
}

uint Proto::mGetRegisterData()
{
    return m_nRegisterData;
}
