#ifndef DACHELPER_H
#define DACHELPER_H
#include "Utils.h"
namespace DacHelper
{
void LxTriggerSetup(iface deviceType,
                  uint32_t trigSourceSelect,
                  uint32_t pwSamples,
                  uint32_t signalDelay,
                  uint32_t pulseGap,
                  uint32_t pulseWidth,
                  uint32_t triggerEnable);

void LxTriggerStart(iface deviceType);
void LxTriggerStop(iface deviceType);

void FixAmplSetting(iface deviceType, int dbm);
void Disable_nco(iface deviceType);
void Enable_nco(iface deviceType);

void WrIterpolation(iface deviceType, int interpoval);
void WrNCOFrq(iface deviceType,QString sNCOFrq);
void NCO_FRQ180Mhz(iface deviceType);
void NCO_FRQ70Mhz(iface deviceType);
void NCO_FRQ60Mhz(iface deviceType);

}
#endif // DACHELPER_H
