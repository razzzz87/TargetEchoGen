#include "Utils.h"
#include "AvrRegAddrDef.h"

namespace  DeviceSetupHelper{

void LmkDefault60MhzSetting(iface deviceType)
{
    // WR1 register write (as your WR1 example)
    //Utils::SpiCtrlWriteReg(deviceType,0x5014, 0x00010000);
    Utils::SpiCtrlWriteReg(deviceType, 0x51c, 0x1);

    // SPI synth register writes
    Utils::WriteSpiSynth(deviceType, 0x00, 0x0000010);
    Utils::WriteSpiSynth(deviceType, 0x01, 0x0000010);
    Utils::WriteSpiSynth(deviceType, 0x02, 0x0000010);
    Utils::WriteSpiSynth(deviceType, 0x03, 0x0000010);
    Utils::WriteSpiSynth(deviceType, 0x04, 0x0000010);
    Utils::WriteSpiSynth(deviceType, 0x05, 0x0000010);
    Utils::WriteSpiSynth(deviceType, 0x06, 0x02808800);
    Utils::WriteSpiSynth(deviceType, 0x07, 0x00408800);
    Utils::WriteSpiSynth(deviceType, 0x08, 0x00888800);
    Utils::WriteSpiSynth(deviceType, 0x0A, 0x048E0210);
    Utils::WriteSpiSynth(deviceType, 0x0B, 0x01BF8880);
    Utils::WriteSpiSynth(deviceType, 0x0C, 0x0098600D);
    Utils::WriteSpiSynth(deviceType, 0x0D, 0x01D81033);
    Utils::WriteSpiSynth(deviceType, 0x0E, 0x00900000);
    Utils::WriteSpiSynth(deviceType, 0x0F, 0x04000400);
    Utils::WriteSpiSynth(deviceType, 0x10, 0x000AA820);
    Utils::WriteSpiSynth(deviceType, 0x18, 0x00000006);
    Utils::WriteSpiSynth(deviceType, 0x19, 0x00080800);
    Utils::WriteSpiSynth(deviceType, 0x1A, 0x057D18E0);
    Utils::WriteSpiSynth(deviceType, 0x1B, 0x008000C1);
    Utils::WriteSpiSynth(deviceType, 0x1C, 0x02EE0180);
    Utils::WriteSpiSynth(deviceType, 0x1D, 0x00000EA6);
    Utils::WriteSpiSynth(deviceType, 0x1E, 0x00000EA6);
}

void LmkDefault120MhzSetting(iface deviceType)
{
    // WR1 register write
    //Utils::SpiCtrlWriteReg(deviceType,0x5014, 0x00010000);
    Utils::SpiCtrlWriteReg(deviceType, 0x51c, 0x0);

    // SPI synth register writes (explicit and ordered for traceability)
    Utils::WriteSpiSynth(deviceType, 0x00, 0x0000010);
    Utils::WriteSpiSynth(deviceType, 0x01, 0x0000010);
    Utils::WriteSpiSynth(deviceType, 0x02, 0x0000010);
    Utils::WriteSpiSynth(deviceType, 0x03, 0x0000010);
    Utils::WriteSpiSynth(deviceType, 0x04, 0x0000010);
    Utils::WriteSpiSynth(deviceType, 0x05, 0x0000010);
    Utils::WriteSpiSynth(deviceType, 0x06, 0x02808800);
    Utils::WriteSpiSynth(deviceType, 0x07, 0x00408800);
    Utils::WriteSpiSynth(deviceType, 0x08, 0x048E0210);
    Utils::WriteSpiSynth(deviceType, 0x0A, 0x048E0210);
    Utils::WriteSpiSynth(deviceType, 0x0B, 0x01BF8880);
    Utils::WriteSpiSynth(deviceType, 0x0C, 0x0098600D);
    Utils::WriteSpiSynth(deviceType, 0x0D, 0x01D81033);
    Utils::WriteSpiSynth(deviceType, 0x0E, 0x00900000);
    Utils::WriteSpiSynth(deviceType, 0x0F, 0x04000400);
    Utils::WriteSpiSynth(deviceType, 0x10, 0x000AA820);
    Utils::WriteSpiSynth(deviceType, 0x18, 0x00000006);
    Utils::WriteSpiSynth(deviceType, 0x19, 0x00080800);
    Utils::WriteSpiSynth(deviceType, 0x1A, 0x047D18E0);
    Utils::WriteSpiSynth(deviceType, 0x1B, 0x008000C1);
    Utils::WriteSpiSynth(deviceType, 0x1C, 0x02EE0180);
    Utils::WriteSpiSynth(deviceType, 0x1D, 0x00000EA6);
    Utils::WriteSpiSynth(deviceType, 0x1E, 0x00000EA6);
}

}
