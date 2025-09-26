#ifndef AVRREGADDRDEF_H
#define AVRREGADDRDEF_H
#include <stdint.h>

namespace AvrTegRegs {
// 1) Block base address for all LMK registers
inline constexpr uint32_t LMK_BASE_ADDR = 0x00001000U;

// 2) Scoped enum of all supported offsets
enum class AvrTegOffset : uint8_t {
    AVR_TEG_ADDR1   = 0x00,
    AVR_TEG_ADDR2   = 0x01,
    AVR_TEG_ADDR3   = 0x02,
    AVR_TEG_ADDR4   = 0x03,
    AVR_TEG_ADDR5   = 0x04,
    AVR_TEG_ADDR6   = 0x05,
    AVR_TEG_ADDR7   = 0x06,
    AVR_TEG_ADDR8   = 0x07,
    AVR_TEG_ADDR9   = 0x08,
    AVR_TEG_ADDR10  = 0x09,
    AVR_TEG_ADDR11  = 0x0A,
    AVR_TEG_ADDR12  = 0x0B,
    AVR_TEG_ADDR13  = 0x0C,
    AVR_TEG_ADDR14  = 0x0D,
    AVR_TEG_ADDR15  = 0x0E,
    AVR_TEG_ADDR16  = 0x0F,
    AVR_TEG_ADDR17  = 0x10,
    AVR_TEG_ADDR18  = 0x18,
    AVR_TEG_ADDR19  = 0x19,
    AVR_TEG_ADDR20  = 0x1A,
    AVR_TEG_ADDR21  = 0x1B,
    AVR_TEG_ADDR22  = 0x1C,
    AVR_TEG_ADDR23  = 0x1D,
    AVR_TEG_ADDR24  = 0x1E
};
// 3) Compute absolute address
inline constexpr uint32_t address(AvrTegOffset off) {
    return LMK_BASE_ADDR + static_cast<uint8_t>(off);
}
}
#endif // AVRREGADDRDEF_H
