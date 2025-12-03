#include "dachelper.h"
#include "log.h"
namespace  DacHelper
{

void LxTriggerSetup(iface deviceType,
                    uint32_t trigSourceSelect,
                    uint32_t pwSamples,
                    uint32_t signalDelay,
                    uint32_t pulseGap,
                    uint32_t pulseWidth,
                    uint32_t triggerEnable)
{
    LOG_INFO("LxTriggerSetup start: deviceType=%d, trigSourceSelect=0x%08X, pwSamples=%u, signalDelay=%u, pulseGap=%u, pulseWidth=%u, triggerEnable=0x%08X",
             static_cast<int>(deviceType),
             trigSourceSelect,
             pwSamples,
             signalDelay,
             pulseGap,
             pulseWidth,
             triggerEnable);

    // Trigger source select (disabled in original; left commented but logged)
    //Utils::RegWrite(deviceType, 0x2024, trigSourceSelect);
    //LOG_INFO("Wrote 0x2024 <- trigSourceSelect=0x%08X", trigSourceSelect);

    // Pulse width samples (disabled in original; left commented but logged)
    //Utils::RegWrite(deviceType, 0x2008, pwSamples);
    //LOG_INFO("Wrote 0x2008 <- pwSamples=%u", pwSamples);

    // Signal delay (disabled in original; left commented but logged)
    //Utils::RegWrite(deviceType, 0x206C, signalDelay);
    //LOG_INFO("Wrote 0x206C <- signalDelay=%u", signalDelay);

    // Pulse gap (PRI)
    Utils::RegWrite(deviceType, 0x2004, pulseGap);
    LOG_INFO("Wrote 0x2004 <- pulseGap=%u", pulseGap);

    // Pulse width
    Utils::RegWrite(deviceType, 0x2000, pulseWidth);
    LOG_INFO("Wrote 0x2000 <- pulseWidth=%u", pulseWidth);

    // Trigger enable
    Utils::RegWrite(deviceType, 0x2018, triggerEnable);
    LOG_INFO("Wrote 0x2018 <- triggerEnable=0x%08X", triggerEnable);

    LOG_INFO("LxTriggerSetup complete for deviceType=%d", static_cast<int>(deviceType));
}


void NCO_FRQ60Mhz(iface deviceType)
{
    LOG_INFO("[NCO_FRQ60Mhz] <ENTER>: deviceType=%d", static_cast<int>(deviceType));
    Utils::SpiDacWrite(deviceType, 0x02, 0xF0C0, 0x02);
    Utils::SpiDacWrite(deviceType, 0x02, 0xF0D0, 0x02);
    Utils::SpiDacWrite(deviceType, 0x14, 0xCCCD, 0x02);
    Utils::SpiDacWrite(deviceType, 0x15, 0x0CCC, 0x02);
    Utils::SpiDacWrite(deviceType, 0x1F, 0x8180, 0x02);
    Utils::SpiDacWrite(deviceType, 0x1F, 0x8182, 0x02);
    Utils::SpiDacWrite(deviceType, 0x1F, 0x8180, 0x02);
    Utils::SpiDacWrite(deviceType, 0x03, 0xF000, 0x02);
    Utils::SpiDacWrite(deviceType, 0x00, 0x819C, 0x00);
    Utils::SpiDacWrite(deviceType, 0x00, 0x819C, 0x01);
    Utils::SpiDacWrite(deviceType, 0x00, 0x019C, 0x02);
}

void NCO_FRQ70Mhz(iface deviceType)
{
    LOG_INFO("[NCO_FRQ70Mhz] <ENTER>: deviceType=%d", static_cast<int>(deviceType));
    Utils::SpiDacWrite(deviceType, 0x02, 0xF0C0, 0x02);
    Utils::SpiDacWrite(deviceType, 0x02, 0xF0D0, 0x02);
    Utils::SpiDacWrite(deviceType, 0x14, 0xEEEF, 0x02);
    Utils::SpiDacWrite(deviceType, 0x15, 0x0EEE, 0x02);
    Utils::SpiDacWrite(deviceType, 0x1F, 0x8180, 0x02);
    Utils::SpiDacWrite(deviceType, 0x1F, 0x8182, 0x02);
    Utils::SpiDacWrite(deviceType, 0x1F, 0x8180, 0x02);
    Utils::SpiDacWrite(deviceType, 0x03, 0xF000, 0x02);
    Utils::SpiDacWrite(deviceType, 0x00, 0x819C, 0x00);
    Utils::SpiDacWrite(deviceType, 0x00, 0x819C, 0x01);
    Utils::SpiDacWrite(deviceType, 0x00, 0x019C, 0x02);

}
void NCO_FRQ180Mhz(iface deviceType){
    LOG_INFO("[NCO_FRQ180Mhz] <ENTER>: deviceType=%d", static_cast<int>(deviceType));
    Utils::SpiDacWrite(deviceType, 0x02, 0xF0C0, 0x02);
    Utils::SpiDacWrite(deviceType, 0x02, 0xF0D0, 0x02);
    Utils::SpiDacWrite(deviceType, 0x14, 0x6666, 0x02);
    Utils::SpiDacWrite(deviceType, 0x15, 0x2666, 0x02);
    Utils::SpiDacWrite(deviceType, 0x1F, 0x8180, 0x02);
    Utils::SpiDacWrite(deviceType, 0x1F, 0x8182, 0x02);
    Utils::SpiDacWrite(deviceType, 0x1F, 0x8180, 0x02);
    Utils::SpiDacWrite(deviceType, 0x03, 0xF000, 0x02);
    Utils::SpiDacWrite(deviceType, 0x00, 0x819C, 0x00);
    Utils::SpiDacWrite(deviceType, 0x00, 0x819C, 0x01);
    Utils::SpiDacWrite(deviceType, 0x00, 0x019C, 0x02);
}

void Enable_nco(iface deviceType)
{
    LOG_INFO("[Enable_nco] <ENTER>: deviceType=%d", static_cast<int>(deviceType));

    uint32_t data = Utils::SpiDacRead(deviceType, 0x02, 0x02);
    LOG_INFO("SpiDacRead: addr=0x02, page=0x02 -> data=0x%08X", data);

    Utils::setBit(data, 4);
    LOG_INFO("setBit: bit=4 -> data=0x%08X", data);

    Utils::SpiDacWrite(deviceType, 0x02, data, 0x02);
    LOG_INFO("SpiDacWrite: addr=0x02, data=0x%08X, page=0x02", data);

    LOG_INFO("[Enable_nco] <EXIT>: deviceType=%d", static_cast<int>(deviceType));
}

void Disable_nco(iface deviceType)
{
    LOG_INFO("[Disable_nco] <ENTER>: deviceType=%d", static_cast<int>(deviceType));

    uint32_t data = Utils::SpiDacRead(deviceType, 0x02, 0x02);
    LOG_INFO("SpiDacRead: addr=0x02, page=0x02 -> data=0x%08X", data);

    Utils::clearBit(data, 4);
    LOG_INFO("clearBit: bit=4 -> data=0x%08X", data);

    Utils::SpiDacWrite(deviceType, 0x02, data, 0x02);
    Utils::SpiDacWrite(deviceType, 0x02, 0xF080, 0x02);
    LOG_INFO("SpiDacWrite: addr=0x02, data=0x%08X, page=0x02", data);

    LOG_INFO("[Disable_nco] <EXIT>: deviceType=%d", static_cast<int>(deviceType));
}


void FixAmplSetting(iface deviceType, int dbm)
{
    switch (dbm)
    {
    case 0:
    case -1:
    case -2:
        Utils::SpiDacWrite(deviceType, 0x3, 0xF000, 0x2);
        break;
    case -3:
        Utils::SpiDacWrite(deviceType, 0x3, 0xE000, 0x2);
        break;
    case -4:
        Utils::SpiDacWrite(deviceType, 0x3, 0xC000, 0x2);
        break;
    case -5:
        Utils::SpiDacWrite(deviceType, 0x3, 0xB000, 0x2);
        break;
    case -6:
        Utils::SpiDacWrite(deviceType, 0x3, 0x9000, 0x2);
        break;
    case -7:
        Utils::SpiDacWrite(deviceType, 0x3, 0x8000, 0x2);
        break;
    case -8:
        Utils::SpiDacWrite(deviceType, 0x3, 0x7000, 0x2);
        break;
    case -9:
        Utils::SpiDacWrite(deviceType, 0x3, 0x6000, 0x2);
        break;
    case -10:
    case -11:
        Utils::SpiDacWrite(deviceType, 0x3, 0x5000, 0x2);
        break;
    case -12:
    case -13:
        Utils::SpiDacWrite(deviceType, 0x3, 0x4000, 0x2);
        break;
    case -14:
    case -15:
        Utils::SpiDacWrite(deviceType, 0x3, 0x3000, 0x2);
        break;
    case -16:
    case -17:
    case -18:
        Utils::SpiDacWrite(deviceType, 0x3, 0x2000, 0x2);
        break;
    case -19:
    case -20:
    case -21:
    case -22:
    case -23:
        Utils::SpiDacWrite(deviceType, 0x3, 0x1000, 0x2);
        break;
    case -24:
    case -25:
    case -26:
    case -27:
        Utils::SpiDacWrite(deviceType, 0x3, 0x0000, 0x2);
        break;
    default:
        // Optionally log unsupported dbm value
        break;
    }
}

void LxTriggerStart(iface deviceType)
{
    LOG_INFO("[LxTriggerStart] <ENTER>: deviceType=%d", static_cast<int>(deviceType));

    Utils::RegWrite(deviceType, 0x201C, 0x01);       // Trigger start
    LOG_INFO("RegisterWrite: addr=0x201C <- 0x%02X", 0x01);

    Utils::RegWrite(deviceType, 0x201C, 0x00);       // Trigger start
    LOG_INFO("RegisterWrite: addr=0x201C <- 0x%02X", 0x00);

    LOG_INFO("[LxTriggerStart] <EXIT>: deviceType=%d", static_cast<int>(deviceType));
}

void LxTriggerStop(iface deviceType)
{
    LOG_INFO("[LxTriggerStop] <ENTER>: deviceType=%d", static_cast<int>(deviceType));

    Utils::RegWrite(deviceType, 0x2018, 0x00);      // Trigger enable/disable
    LOG_INFO("RegisterWrite: addr=0x2018 <- 0x%02X", 0x00);

    LOG_INFO("[LxTriggerStop] <EXIT>: deviceType=%d", static_cast<int>(deviceType));
}

void WrIterpolation(iface deviceType,int interpoval)
{
    switch(interpoval)
    {
    case 2:
        Utils::RegWrite(deviceType,AVR_TEG_REG_BASE+0x38,0x01);
        break;
    case 4:
        Utils::RegWrite(deviceType,AVR_TEG_REG_BASE+0x38,0x02);
        break;
    case 8:
        Utils::RegWrite(deviceType,AVR_TEG_REG_BASE+0x38,0x03);
        break;
    default:
        Utils::RegWrite(deviceType,AVR_TEG_REG_BASE+0x38,0x00);
    }
    Utils::RegWrite(deviceType,AVR_TEG_REG_BASE+0x34,0x01);
    Utils::RegWrite(deviceType,AVR_TEG_REG_BASE+0x34,0x00);

}
void WrNCOFrq(iface deviceType,QString sNCOFrq)
{
    if(sNCOFrq == "70MHz"){
        //Utils::RegWrite(deviceType,AVR_TEG_REG_BASE+0x38,0x01);
    }
    else if(sNCOFrq == "180MHz"){
        //Utils::RegWrite(deviceType,AVR_TEG_REG_BASE+0x38,0x01);
    }else{
        //Utils::RegWrite(deviceType,AVR_TEG_REG_BASE+0x38,0x01);
    }
}
void IQSwap(iface deviceType,QString sSwapIQ)
{
    if(sSwapIQ == "Normal"){
        Utils::RegWrite(deviceType,AVR_DAC3_BASE_ADDR+0x4C,0);
    }
    else if(sSwapIQ == "Swap"){
        Utils::RegWrite(deviceType,AVR_DAC3_BASE_ADDR+0x4C,1);
    }else{
        Utils::RegWrite(deviceType,AVR_DAC3_BASE_ADDR+0x4C,0);
    }

}
}
