#ifndef DEVICESETUPHELPER_H
#define DEVICESETUPHELPER_H
#include "Utils.h"
#include "AvrRegAddrDef.h"
namespace DeviceSetupHelper {
// uint32_t AVRRegAccessRead(iface deviceType,AvrTegRegs::AvrTegOffset off);
// uint32_t AVRRegAccessWrite(iface deviceType, AvrTegRegs::AvrTegOffset off, uint32_t propValue);
// uint32_t LMKRegAccessWrite(iface deviceType, AvrTegRegs::AvrTegOffset regOffset, unsigned int propValue);
// uint32_t ProgramDefaultInLMKRegisters(iface deviceType, unsigned int moduleID);
void LMKDefaultSetting(iface deviceType);
void LmkDefault60MhzSetting(iface deviceType);
void LmkDefault120MhzSetting(iface deviceType);
//void WriteSpiSynth(iface deviceType, uint32_t data, uint32_t address, uint32_t numPostClock, bool syncEnAuto);
}
#endif // DEVICESETUPHELPER_H
