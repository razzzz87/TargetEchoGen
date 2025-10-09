#ifndef MAINWINDOWHELPER_H
#define MAINWINDOWHELPER_H
#include "Utils.h"
namespace MainWindowHelper
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

}
#endif // MAINWINDOWHELPER_H
