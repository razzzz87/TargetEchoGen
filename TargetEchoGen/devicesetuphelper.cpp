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
void Dac2DefaultSetting(iface deviceType)
{
    Utils::SpiDacWrite(deviceType, 0x00, 0x019C,0x2);
    Utils::SpiDacWrite(deviceType, 0x01, 0x100E,0x2);
    Utils::SpiDacWrite(deviceType, 0x02, 0xF080,0x2);
    Utils::SpiDacWrite(deviceType, 0x03, 0xF000,0x2);
    Utils::SpiDacWrite(deviceType, 0x04, 0xFDFD,0x2);
    Utils::SpiDacWrite(deviceType, 0x05, 0x3900,0x2);
    Utils::SpiDacWrite(deviceType, 0x06, 0x3D00,0x2);
    Utils::SpiDacWrite(deviceType, 0x07, 0x0000,0x2);
    Utils::SpiDacWrite(deviceType, 0x08, 0x0000,0x2);
    Utils::SpiDacWrite(deviceType, 0x09, 0x8000,0x2);
    Utils::SpiDacWrite(deviceType, 0x0A, 0x0000,0x2);
    Utils::SpiDacWrite(deviceType, 0x0B, 0x0000,0x2);
    Utils::SpiDacWrite(deviceType, 0x0C, 0x0400,0x2);
    Utils::SpiDacWrite(deviceType, 0x0D, 0x0400,0x2);
    Utils::SpiDacWrite(deviceType, 0x0E, 0x0400,0x2);
    Utils::SpiDacWrite(deviceType, 0x0F, 0x0400,0x2);
    Utils::SpiDacWrite(deviceType, 0x10, 0x3000,0x2);
    Utils::SpiDacWrite(deviceType, 0x11, 0x0000,0x2);
    Utils::SpiDacWrite(deviceType, 0x12, 0x0000,0x2);
    Utils::SpiDacWrite(deviceType, 0x13, 0x0000,0x2);
    Utils::SpiDacWrite(deviceType, 0x14, 0x0000,0x2);
    Utils::SpiDacWrite(deviceType, 0x15, 0x0000,0x2);
    Utils::SpiDacWrite(deviceType, 0x16, 0x0000,0x2);
    Utils::SpiDacWrite(deviceType, 0x17, 0x0000,0x2);
    Utils::SpiDacWrite(deviceType, 0x18, 0x2456,0x2);
    Utils::SpiDacWrite(deviceType, 0x19, 0x0804,0x2);
    Utils::SpiDacWrite(deviceType, 0x1A, 0x7000,0x2);
    Utils::SpiDacWrite(deviceType, 0x1B, 0x0000,0x2);
    Utils::SpiDacWrite(deviceType, 0x1C, 0x0000,0x2);
    Utils::SpiDacWrite(deviceType, 0x1D, 0x0000,0x2);
    Utils::SpiDacWrite(deviceType, 0x1E, 0x1111,0x2);
    Utils::SpiDacWrite(deviceType, 0x1F, 0x1140,0x2);
    Utils::SpiDacWrite(deviceType, 0x20, 0x2201,0x2);
    Utils::SpiDacWrite(deviceType, 0x22, 0x1B1B,0x2);
    Utils::SpiDacWrite(deviceType, 0x23, 0xFFFF,0x2);
    Utils::SpiDacWrite(deviceType, 0x24, 0x0800,0x2);
    Utils::SpiDacWrite(deviceType, 0x25, 0x7A7A,0x2);
    Utils::SpiDacWrite(deviceType, 0x26, 0xB6B6,0x2);
    Utils::SpiDacWrite(deviceType, 0x27, 0xEAEA,0x2);
    Utils::SpiDacWrite(deviceType, 0x28, 0x4545,0x2);
    Utils::SpiDacWrite(deviceType, 0x29, 0x1A1A,0x2);
    Utils::SpiDacWrite(deviceType, 0x2A, 0x1616,0x2);
    Utils::SpiDacWrite(deviceType, 0x2B, 0xAAAA,0x2);
    Utils::SpiDacWrite(deviceType, 0x2C, 0xC6C6,0x2);
    Utils::SpiDacWrite(deviceType, 0x2D, 0x0000,0x2);
    Utils::SpiDacWrite(deviceType, 0x2E, 0x0000,0x2);
    Utils::SpiDacWrite(deviceType, 0x2F, 0x0000,0x2);
    Utils::SpiDacWrite(deviceType, 0x30, 0x0000,0x2);
    Utils::SpiDacWrite(deviceType, 0x7F, 0x540C,0x2);
}

}
