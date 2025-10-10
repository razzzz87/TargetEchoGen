#include "mainwindowhelper.h"

namespace  MainWindowHelper
{

void LxTriggerSetup(iface deviceType,
                  uint32_t trigSourceSelect,
                  uint32_t pwSamples,
                  uint32_t signalDelay,
                  uint32_t pulseGap,
                  uint32_t pulseWidth,
                  uint32_t triggerEnable)
{
    //Utils::RegisterWrite(deviceType, 0x2024, trigSourceSelect);  // Trigger source select
    //Utils::RegisterWrite(deviceType, 0x2008, pwSamples);          // Pulse width samples
    //Utils::RegisterWrite(deviceType, 0x206C, signalDelay);        // Signal delay
    Utils::RegisterWrite(deviceType, 0x2004, pulseGap);           // Pulse gap (PRI)
    Utils::RegisterWrite(deviceType, 0x2000, pulseWidth);         // Pulse width
    Utils::RegisterWrite(deviceType, 0x2018, triggerEnable);      // Trigger enable
}

void Enable_nco(iface deviceType)
{
    uint32_t data = Utils::SpiDacRead(deviceType, 0x02, 0x02);
    Utils::setBit(data,4);
    Utils::SpiDacWrite(deviceType,0x2,data,0x2);

}

void Disable_nco(iface deviceType)
{
    uint32_t data = Utils::SpiDacRead(deviceType, 0x02, 0x02);
    Utils::clearBit(data,4);
    Utils::SpiDacWrite(deviceType,0x2,data,0x2);
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
    Utils::RegisterWrite(deviceType, 0x201C, 0x01);       // Trigger start
    Utils::RegisterWrite(deviceType, 0x201C, 0x00);       // Trigger start
}
void LxTriggerStop(iface deviceType)
{
    Utils::RegisterWrite(deviceType, 0x2018,0x00);      // Trigger enable/disable
}

}
