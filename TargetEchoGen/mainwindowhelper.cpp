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
