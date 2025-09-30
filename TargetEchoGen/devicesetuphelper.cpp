#include "Utils.h"
#include "AvrRegAddrDef.h"

namespace  DeviceSetupHelper{

long ProgramDefaultInLMKRegisters(iface deviceType, unsigned int moduleID)
{
    // unsigned int RegLmkFreq = 0;
    // // Always apply the default 120 MHz LMK map
    // switch(RegLmkFreq){
    // case 1:
    //     LOG_INFO("**** DEFAULT SETTINGS FOR LMK @ 120 MHz ****");
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR1,  0x0000010);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR2,  0x0000010);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR3,  0x0000010);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR4,  0x0000010);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR5,  0x0000010);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR6,  0x0000010);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR7,  0x02808800);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR8,  0x00408800);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR9,  0x048E0210);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR10, 0x0888800);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR11, 0x48E0210);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR12, 0x1BF8880);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR13, 0x098600D);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR14, 0x1D81033);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR15, 0x0900000);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR16, 0x4000400);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR17, 0x00AA820);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR18, 0x00000006);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR19, 0x0080800);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR20, 0x047D18E0);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR21, 0x08000C1);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR22, 0x2EE0180);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR23, 0x0000EA6);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR24, 0x0000EA6);
    // case 2:
    //     LOG_INFO("**** DEFAULT SETTINGS FOR LMK @ 120 MHz ****");
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR1,  0x0000010);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR2,  0x0000010);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR3,  0x0000010);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR4,  0x0000010);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR5,  0x0000010);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR6,  0x0000010);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR7,  0x02808800);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR8,  0x00408800);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR9,  0x048E0210);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR10, 0x0888800);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR11, 0x48E0210);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR12, 0x1BF8880);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR13, 0x098600D);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR14, 0x1D81033);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR15, 0x0900000);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR16, 0x4000400);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR17, 0x00AA820);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR18, 0x00000006);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR19, 0x0080800);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR20, 0x047D18E0);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR21, 0x08000C1);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR22, 0x2EE0180);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR23, 0x0000EA6);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR24, 0x0000EA6);
    // case 3:
    //     LOG_INFO("**** DEFAULT SETTINGS FOR LMK @ 120 MHz ****");
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR1,  0x0000010);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR2,  0x0000010);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR3,  0x0000010);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR4,  0x0000010);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR5,  0x0000010);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR6,  0x0000010);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR7,  0x02808800);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR8,  0x00408800);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR9,  0x048E0210);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR10, 0x0888800);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR11, 0x48E0210);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR12, 0x1BF8880);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR13, 0x098600D);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR14, 0x1D81033);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR15, 0x0900000);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR16, 0x4000400);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR17, 0x00AA820);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR18, 0x00000006);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR19, 0x0080800);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR20, 0x047D18E0);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR21, 0x08000C1);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR22, 0x2EE0180);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR23, 0x0000EA6);
    //     LMKRegAccessWrite(deviceType, AvrTegRegs::AvrTegOffset::AVR_TEG_ADDR24, 0x0000EA6);
    // default:
    //     LOG_INFO("Invalid value\n");
    // }
    // return 0;
}

void LMKDefaultSetting(iface deviceType)
{
    Utils::RegisterWrite(deviceType, 0x00005014, 0x00000000);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00000000);
    Utils::RegisterWrite(deviceType, 0x00001010, 0x00000201);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00000400);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00000401);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00000800);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00000801);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00000C00);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00000C01);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00001000);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00001001);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00001400);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00001401);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00001800);
    Utils::RegisterWrite(deviceType, 0x00001010, 0x50110001);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000180C);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000980C);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000980D);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x00009C0C);
    Utils::RegisterWrite(deviceType, 0x00001010, 0x08110001);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000A00C);
    Utils::RegisterWrite(deviceType, 0x00001010, 0x91C04201);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000A00D);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000A80C);
    Utils::RegisterWrite(deviceType, 0x00001010, 0x91C04201);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000A80D);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000AC0C);
    Utils::RegisterWrite(deviceType, 0x00001010, 0x37F11001);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000AC0D);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000B00C);
    Utils::RegisterWrite(deviceType, 0x00001010, 0x130C01A1);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000B00D);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000B40C);
    Utils::RegisterWrite(deviceType, 0x00001010, 0x3B020661);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000B40D);
    Utils::RegisterWrite(deviceType, 0x0000100C, 0x0000B80C);
    Utils::RegisterWrite(deviceType, 0x00001010, 0x12000001);
}

void writeSpiSynth(iface deviceType, uint32_t data, uint32_t address, uint32_t numPostClock, bool syncEnAuto)
{
    uint32_t val;

    // 1) Set 5-bit ADDRESS field in reg 0x100C (bits [14:10])
    val = Utils::readRegisterValue(deviceType, 0x100C);
    // clear bits 10–14, then OR in (address & 0x1F) << 10
    val = (val & 0xFFFF83FFu)| ((address     & 0x1Fu) << 10);
    Utils::RegisterWrite(deviceType, 0x100C, val);

    // 2) Set 27-bit DATA field in reg 0x1010 (bits [31:5])
    val = Utils::readRegisterValue(deviceType, 0x1010);
    // clear bits 5–31, then OR in (data & 0x07FFFFFF) << 5
    val = (val & 0x0000001Fu)| ((data        & 0x07FFFFFFu) << 5);
    Utils::RegisterWrite(deviceType, 0x1010, val);

    // 3) Set NUM_POSTCLOCK in reg 0x100C (bits [9:2])
    val = Utils::readRegisterValue(deviceType, 0x100C);
    // clear bits 2–9, then OR in (numPostClock & 0xFF) << 2
    val = (val & 0xFFFFFC03u) | ((numPostClock & 0xFFu) << 2);
    Utils::RegisterWrite(deviceType, 0x100C, val);

    // 4) Set SYNC_EN_AUTO in reg 0x100C (bit 15)
    val = Utils::readRegisterValue(deviceType, 0x100C);
    // clear bit 15, then OR in (syncEnAuto & 1) << 15
    val = (val & 0xFFFF7FFFu) | (((uint32_t)syncEnAuto & 0x1u) << 15);
    Utils::RegisterWrite(deviceType, 0x100C, val);

    // 5) Toggle SPI-WRITE strobe (bit 0) in reg 0x100C
    val = Utils::readRegisterValue(deviceType, 0x100C);
    Utils::RegisterWrite(deviceType, 0x100C, val | 0x1u);     // set bit 0
    val = Utils::readRegisterValue(deviceType, 0x100C);
    Utils::RegisterWrite(deviceType, 0x100C, val & ~0x1u);    // clear bit 0
}

}
