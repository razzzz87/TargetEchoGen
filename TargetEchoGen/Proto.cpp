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
}

ushort Proto::mPktRegWrite(uint a_nRegisterAddress, uint a_nRegisterData, char** a_byArrPacket)
{
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

    // Packet Length (2 bytes)
    ushort nLen = PACKET_SIZE;
    uchar* pchPktLen = (uchar*)&nLen;
    mMemCpy(byArrPacket + m_nPacketLength, pchPktLen, 2);
    m_nPacketLength += 2;

    // Command (2 bytes)
    ushort cmd = CMD_REG_WRITE;
    uchar* pchCmd = (uchar*)&cmd;
    mMemCpy(byArrPacket + m_nPacketLength, pchCmd, 2);
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
}

ushort Proto::mPktBulkRead(uint a_nRegisterAddress, uint a_nPacketLen, char **a_byArrPacket)
{
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
}

ushort Proto::mPktRegRead(uint a_nRegisterAddress, char **a_byArrPacket)
{
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

    // Length (2 bytes)
    ushort nLen = PACKET_SIZE;
    uchar *pchPktLen = (uchar *)&nLen;
    mMemCpy(byArrPacket + m_nPacketLength, pchPktLen, 2);
    m_nPacketLength += 2;

    // Command (2 bytes)
    ushort cmd = CMD_REG_READ;
    uchar *pchCmd = (uchar *)&cmd;
    mMemCpy(byArrPacket + m_nPacketLength, pchCmd, 2);
    m_nPacketLength += 2;

    // Register Address (4 bytes)
    uchar *pchRegAddr = (uchar *)&a_nRegisterAddress;
    mMemCpy(byArrPacket + m_nPacketLength, pchRegAddr, 4);
    m_nPacketLength += 4;

    // Padding (4 bytes)
    memset(byArrPacket + m_nPacketLength, '\0', 4);
    m_nPacketLength += 4;

    return m_nPacketLength;
}

ushort Proto::mPktBulkReadUART(uint a_nRegisterAddress, uint a_nRegData, uint a_nPacketLen, char **a_byArrPacket)
{
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

/*ushort Proto::mPktAction(Proto::ACTION_TYPE a_enActionType, QByteArray &byArrPacket)
{
    m_nPacketLength =   0;

    ushort nMagicByte =   SEND_MAGIC_BYTE_new;
    uchar *pchMagicByte;
    pchMagicByte =  (uchar *) &nMagicByte;

    //send magic bytes (2 bytes)
    byArrPacket.append(pchMagicByte[0]);
    byArrPacket.append(pchMagicByte[1]);
    m_nPacketLength +=  2;

    //length bytes (2 bytes)
    byArrPacket.append('\0');
    byArrPacket.append('\0');
    m_nPacketLength +=  2;

    //cmnd bytes (4 bytes)
    byArrPacket.append('\0');
    byArrPacket.append(CMD_ACTION);     //0x1:RW, 0x2:RR, 0x3:Action
    byArrPacket.append('\0');
    byArrPacket.append('\0');
    m_nPacketLength +=  4;

    unsigned char chActionType  =   0x01    <<  (int)a_enActionType;
    byArrPacket.append(chActionType);
    byArrPacket.append('\0');
    byArrPacket.append('\0');
    byArrPacket.append('\0');
    m_nPacketLength +=  4;

    byArrPacket.append('\0');
    byArrPacket.append('\0');
    byArrPacket.append('\0');
    byArrPacket.append('\0');
    m_nPacketLength +=  4;

    uchar *pchPktLen;
    pchPktLen   =   (uchar *)&m_nPacketLength;
    byArrPacket[2]  =   pchPktLen[0];
    byArrPacket[3]  =   pchPktLen[1];

    return m_nPacketLength;
}

ushort Proto::mPktSetProp(Proto::SET_PROP a_enProp, uint a_nValue, QByteArray &byArrPacket)
{
    m_nPacketLength =   0;

    ushort nMagicByte =   SEND_MAGIC_BYTE_new;
    uchar *pchMagicByte;
    pchMagicByte =  (uchar *) &nMagicByte;

    //send magic bytes (2 bytes)
    byArrPacket.append(pchMagicByte[0]);
    byArrPacket.append(pchMagicByte[1]);
    m_nPacketLength +=  2;

    //length bytes (2 bytes)
    byArrPacket.append('\0');
    byArrPacket.append('\0');
    m_nPacketLength +=  2;

    //cmnd bytes (4 bytes)
    byArrPacket.append('\0');
    byArrPacket.append(CMD_GET_PROP);     //0x1:RW, 0x2:RR, 0x3:Action
    byArrPacket.append('\0');
    byArrPacket.append('\0');
    m_nPacketLength +=  4;

    uchar *pchPropType;
    uint nPropType      =   (int)a_enProp;
    pchPropType  =   (uchar *)&nPropType;
    byArrPacket.append(pchPropType[0]);
    byArrPacket.append(pchPropType[1]);
    byArrPacket.append(pchPropType[2]);
    byArrPacket.append(pchPropType[3]);
    m_nPacketLength +=  4;

    uchar *pchData;
    pchData  =   (uchar *)&a_nValue;
    byArrPacket.append(pchData[0]);
    byArrPacket.append(pchData[1]);
    byArrPacket.append(pchData[2]);
    byArrPacket.append(pchData[3]);
    m_nPacketLength +=  4;

    uchar *pchPktLen;
    pchPktLen   =   (uchar *)&m_nPacketLength;
    byArrPacket[2]  =   pchPktLen[0];
    byArrPacket[3]  =   pchPktLen[1];

    return m_nPacketLength;
}

ushort Proto::mPktReset(QByteArray &byArrPacket)
{

}*/
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

    if((m_nMagicBytes    ==  SEND_MAGIC_BYTE_new) |   (m_nMagicBytes   ==  RECV_MAGIC_BYTE_new))
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

    // Extract packet length (2 bytes, big endian)
    uint16_t m_nPacketLength = (static_cast<uint8_t>(byArrPktResp[nParser]) << 8) |
                               static_cast<uint8_t>(byArrPktResp[nParser + 1]);
    nParser += 2;

    if (m_nPacketLength != 16) return 0;

    // Extract command (2 bytes, if applicable)
    uint16_t m_nCommand = (static_cast<uint8_t>(byArrPktResp[nParser]) << 8) |
                          static_cast<uint8_t>(byArrPktResp[nParser + 1]);
    nParser += 2;

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
}

int Proto::mPktParseBulkRead(char *a_byArrData)
{
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
    if((m_nMagicBytes    ==  SEND_MAGIC_BYTE_new) |   (m_nMagicBytes   ==  RECV_MAGIC_BYTE_new))
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

    return 0; //successful
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
