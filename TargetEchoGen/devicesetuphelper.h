#ifndef DEVICESETUPHELPER_H
#define DEVICESETUPHELPER_H
#include "Utils.h"

enum DeviceType
{
    LMX,
    LMK,
    FPGA,
    DAC1,
    DAC2,
    DAC3,
    ATTN1,
    ATTN2,
    ATTN3,
    ATTN4,
    COUNT // it should be always last
};

namespace DeviceSetupHelper
{
    QString DeviceTypeToQStringDirect(DeviceType d);
    void LmkDefault60MhzSetting(iface deviceType);
    void LmkDefault120MhzSetting(iface deviceType);
    void Dac3DefaultSetting(iface deviceType);
}
#endif // DEVICESETUPHELPER_H
