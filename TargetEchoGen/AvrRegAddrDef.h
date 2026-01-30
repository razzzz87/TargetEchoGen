#ifndef AVR_SPI_CONTROL_REG_H
#define AVR_SPI_CONTROL_REG_H

// SPI Control Interface
#include <cstdint>
#define AVR_SPI_CTRL_BASE_ADDR             0x00001000UL

// DDR3 Test Data Generator/Checker
#define AVR_DDR3_TDG_BASE_ADDR             0x80000000UL

// Software DDR3 Read/Write Adapter Block
#define AVR_DDR3_RW_ADAPTER_BASE_ADDR      0x80000100UL

// DAC Interface
#define AVR_DAC_INTERFACE_BASE_ADDR        0x80002000UL

// DAC0 DDR3 Adapter Block
#define AVR_DAC0_DDR3_ADAPTER_BASE_ADDR    0x80002100UL

// DAC1 DDR3 Adapter Block
#define AVR_DAC1_DDR3_ADAPTER_BASE_ADDR    0x80002140UL

// LVDS Interface
#define AVR_LVDS_INTERFACE_BASE_ADDR       0x80002200UL

// SPI Flash Interface
// (as given: 0x8000_3000_0240)
#define AVR_SPI_FLASH_BASE_ADDR            0x800030000240UL

// Manufacturing Control & Status
#define AVR_MANUF_CTRL_STATUS_BASE_ADDR    0x80004000UL

// I2C Slave Control (DS1721)
#define AVR_I2C_SLAVE_CTRL_BASE_ADDR       0x80004100UL

// Common Interface Registers
#define AVR_INTERFACE_BASE_ADDR            0x80004200UL

// Clock & Reset Control
#define AVR_CLK_RST_CTRL_BASE_ADDR         0x00005000UL

//DAC1
// #define AVR_TEG_REG_BASE 0x0000500

//DAC3
#define AVR_DAC3_BASE_ADDR  0x00002000UL


// SPI Control Interface Register Offsets
typedef enum {
    // SPI DAC Interface
    AVR_SPI_DAC_WRITE_ENABLE        = 0x00, // Write enable signal for master (Pulse)
    AVR_SPI_DAC_READ_ENABLE         = 0x04, // Read enable signal for master (Pulse)
    AVR_SPI_DAC_SELECT              = 0x08, // DAC selection: 00–DAC0, 01–DAC1, 10–DAC2
    AVR_SPI_DAC_ADDRESS             = 0x0C, // Register address of SPI slave device
    AVR_SPI_DAC_DATA_IN             = 0x10, // Data to be written to slave register
    AVR_SPI_DAC_READY               = 0x14, // Master ready signal (RO)
    AVR_SPI_DAC_DATA_OUT_VALID      = 0x18, // Output data validity signal (RO)
    AVR_SPI_DAC_DATA_OUT            = 0x1C, // Data output from slave register (RO)

    // Attenuator Interface
    AVR_ATTN_WRITE_ENABLE           = 0x20, // Write enable signal for master (Pulse)
    AVR_ATTN_DEV_SELECT             = 0x24, // Attenuator device select: 0–Attn0, 1–Attn1
    AVR_ATTN_READY                  = 0x28, // Master ready signal (RO)
    AVR_ATTN_ADDRESS                = 0x2C, // Register address of SPI slave device
    AVR_ATTN_DATA_IN                = 0x30, // Data to be written to slave register

    // Clock Synthesizer Interface
    AVR_CLK_WRITE_ENABLE            = 0x34, // Write enable signal for master (Pulse)
    AVR_CLK_READ_ENABLE             = 0x38, // Read enable signal for master (Pulse)
    AVR_CLK_NUM_POSTCLOCK           = 0x3C, // Postclock count based on address
    AVR_CLK_ADDRESS                 = 0x40, // Register address of SPI slave device
    AVR_CLK_SYNC_EN_AUTO            = 0x44, // Sync enable auto based on address
    AVR_CLK_DATA_IN                 = 0x48, // Data to be written to slave register
    AVR_CLK_STATUS_LD               = 0x4C, // PLL lock status of LMK chip (RO)
    AVR_CLK_READY                   = 0x50, // Master ready signal (RO)
    AVR_CLK_DATA_OUT_VALID          = 0x54, // Output data validity signal (RO)
    AVR_CLK_DATA_OUT                = 0x58  // Data output from slave register (RO)
} AVR_SPI_Control_Reg;

// DDR3 Test Data Generator/Checker Register Offsets
typedef enum {
    // Write Size Register
    AVR_DDR3_TDG_WRITE_SIZE     = 0x00, // RW, bits[31:0], number of bytes to write

    // Read Size Register
    AVR_DDR3_TDG_READ_SIZE      = 0x04, // RW, bits[31:0], number of bytes to read

    // Fixed Data Pattern Register
    AVR_DDR3_TDG_FIXED_PATTERN  = 0x08, // RW, bits[31:0], 32-bit fixed data pattern for write & read

    // Control Register
    //   bits[1:0]   Data Type: 00-Fixed, 01-Incremental, 10-PRBS, 11-Reserved
    //   bit[2]      Send Data (pulse)
    //   bit[3]      Read Data (pulse)
    //   bit[4]      Checker Enable
    AVR_DDR3_TDG_CTRL           = 0x0C, // RW

    // Status & Version Register
    //   bit[0]      DDR Init Complete (RO)
    //   bit[1]      Write Done    (RO)
    //   bit[2]      Read Done     (RO)
    //   bit[3]      Checker Result (0: no error, 1: error)
    //   bits[31:24] Version Reg   (RO)
    AVR_DDR3_TDG_STATUS         = 0x10  // RO
} AVR_DDR3_TDG_Reg;

// Software DDR3 Read/Write Adapter Block Register Offsets
typedef enum {
    // Write Start Address Register
    AVR_DDR3_RW_ADAPTER_WRITE_START_ADDR   = 0x00, // RW, bits[31:0], DDR3 write start burst address

    // Write Wrap Address Register
    AVR_DDR3_RW_ADAPTER_WRITE_WRAP_ADDR    = 0x04, // RW, bits[31:0], DDR3 write wrap burst address (bytes/64)

    // Write Size Register
    AVR_DDR3_RW_ADAPTER_WRITE_SIZE         = 0x08, // RW, bits[31:0], number of bytes to write

    // Read Start Address Register
    AVR_DDR3_RW_ADAPTER_READ_START_ADDR    = 0x0C, // RW, bits[31:0], DDR3 read start burst address

    // Read Wrap Address Register
    AVR_DDR3_RW_ADAPTER_READ_WRAP_ADDR     = 0x10, // RW, bits[31:0], DDR3 read wrap burst address (bytes/64)

    // Read Size Register
    AVR_DDR3_RW_ADAPTER_READ_SIZE          = 0x14, // RW, bits[31:0], number of bytes to read

    // Control Register
    //   bits[7:0]   AXI Burst Length
    //   bit[8]      Write Indefinite
    //   bit[9]      Read Indefinite
    //   bit[10]     Send Data Regif (write start pulse)
    //   bit[11]     Read Data Regif (read start pulse)
    AVR_DDR3_RW_ADAPTER_CONTROL            = 0x18, // RW

    // Status Register
    //   bit[0]      DDR Initialization Complete (RO)
    //   bit[1]      Write Done Flag            (RO)
    //   bit[2]      Read Done Flag             (RO)
    AVR_DDR3_RW_ADAPTER_STATUS             = 0x1C  // RO
} AVR_DDR3_RW_Adapter_Reg;

// DAC Interface Register Offsets
typedef enum {
    // Trigger Pulse Width Register
    AVR_DAC_TRIG_PULSE_WIDTH       = 0x00, // RW, bits[31:0], width of the trigger pulse in DAC clock cycles

    // Trigger Pulse Gap Register
    AVR_DAC_TRIG_PULSE_GAP         = 0x04, // RW, bits[31:0], gap between two trigger pulses in DAC clock cycles

    // DAC0 Data Count Per Trigger Register
    AVR_DAC0_COUNT_PER_TRIG        = 0x08, // RW, bits[31:0], output data count for DAC0 per trigger

    // Control Register
    //   bit 0: Do DDR Reading For DAC0 (pulse)
    //   bit 1: SW Trigger Enable
    //   bit 2: SW Trigger Start (pulse)
    //   bit 3: Do DDR Reading For DAC1 (pulse)
    //   bit 4: Trigger_In_check_pulse
    //   bit 5: Trigger_In_check_result (RO)
    //   bit 6: Trigger Source Select (0=J14 connector, 1=SMA connector)
    AVR_DAC_CONTROL                = 0x0C, // RW

    // Error Flag FIFO Empty Register
    //   bit 0: DAC0 Error Flag FIFO Empty (RO)
    //   bit 1: DAC1 Error Flag FIFO Empty (RO)
    AVR_DAC_ERROR_FLAG_FIFO_EMPTY  = 0x10, // RO

    // DAC1 Data Count Per Trigger Register
    AVR_DAC1_COUNT_PER_TRIG        = 0x14, // RW, bits[31:0], output data count for DAC1 per trigger

    // Analog Board Control Register
    //   bit 0 : DAC_0_Chip_Reset       (active low)
    //   bit 1 : DAC_1_Chip_Reset       (active low)
    //   bit 2 : DAC_2_Chip_Reset       (active low)
    //   bit 3 : DAC_0_Sleep            (active high)
    //   bit 4 : DAC_1_Sleep            (active high)
    //   bit 5 : DAC_2_Sleep            (active high)
    //   bit 6 : DAC_0_TX_Enable        (active high)
    //   bit 7 : DAC_1_TX_Enable        (active high)
    //   bit 8 : DAC_2_TX_Enable        (active high)
    //   bit 9 : IQ_MOD_0_Power_Down    (1=Off, 0=On)
    //   bit 10: IQ_MOD_1_Power_Down    (1=Off, 0=On)
    //   bit 11: IQ_MOD_0_Gain_Control  (1=High Gain, 0=Normal)
    //   bit 12: IQ_MOD_1_Gain_Control  (1=High Gain, 0=Normal)
    //   bit 13: LMK_Lock_Detect        (RO)
    AVR_DAC_ANALOG_BOARD_CTRL      = 0x18, // RW

    // Delay With Respect to Rising Edge Register
    AVR_DAC_DELAY_WRT_RISING_EDGE  = 0x1C  // RW, bits[31:0], delay count in DAC clock cycles
} AVR_DAC_Interface_Reg;

// DAC0 DDR3 Adapter Block Register Offsets
typedef enum {
    // Write Start Address Register
    AVR_DAC0_DDR3_ADAPTER_WRITE_START_ADDR = 0x00, // RW, bits[31:0], DDR3 write start burst address

    // Write Wrap Address Register
    AVR_DAC0_DDR3_ADAPTER_WRITE_WRAP_ADDR  = 0x04, // RW, bits[31:0], DDR3 write wrap burst address (bytes/64)

    // Write Size Register
    AVR_DAC0_DDR3_ADAPTER_WRITE_SIZE       = 0x08, // RW, bits[31:0], number of bytes to write

    // Read Start Address Register
    AVR_DAC0_DDR3_ADAPTER_READ_START_ADDR  = 0x0C, // RW, bits[31:0], DDR3 read start burst address

    // Read Wrap Address Register
    AVR_DAC0_DDR3_ADAPTER_READ_WRAP_ADDR   = 0x10, // RW, bits[31:0], DDR3 read wrap burst address (bytes/64)

    // Read Size Register
    AVR_DAC0_DDR3_ADAPTER_READ_SIZE        = 0x14, // RW, bits[31:0], number of bytes to read

    // Control Register
    //   bits[7:0]   AXI Burst Length
    //   bit[8]      Write Indefinite
    //   bit[9]      Read Indefinite
    //   bit[10]     Send Data Regif (write start pulse)
    //   bit[11]     Read Data Regif (read start pulse)
    AVR_DAC0_DDR3_ADAPTER_CONTROL          = 0x18, // RW

    // Status Register
    //   bit[0]      DDR Initialization Complete (RO)
    //   bit[1]      Write Done Flag             (RO)
    //   bit[2]      Read Done Flag              (RO)
    AVR_DAC0_DDR3_ADAPTER_STATUS           = 0x1C  // RO
} AVR_DAC0_DDR3_Adapter_Reg;

// DAC1 DDR3 Adapter Block Register Offsets
typedef enum {
    // Write Start Address Register
    AVR_DAC1_DDR3_ADAPTER_WRITE_START_ADDR = 0x00, // RW, bits[31:0], DDR3 write start burst address

    // Write Wrap Address Register
    AVR_DAC1_DDR3_ADAPTER_WRITE_WRAP_ADDR  = 0x04, // RW, bits[31:0], DDR3 write wrap burst address (bytes/64)

    // Write Size Register
    AVR_DAC1_DDR3_ADAPTER_WRITE_SIZE       = 0x08, // RW, bits[31:0], number of bytes to write

    // Read Start Address Register
    AVR_DAC1_DDR3_ADAPTER_READ_START_ADDR  = 0x0C, // RW, bits[31:0], DDR3 read start burst address

    // Read Wrap Address Register
    AVR_DAC1_DDR3_ADAPTER_READ_WRAP_ADDR   = 0x10, // RW, bits[31:0], DDR3 read wrap burst address (bytes/64)

    // Read Size Register
    AVR_DAC1_DDR3_ADAPTER_READ_SIZE        = 0x14, // RW, bits[31:0], number of bytes to read

    // Control Register
    //   bits[7:0]   AXI Burst Length
    //   bit[8]      Write Indefinite
    //   bit[9]      Read Indefinite
    //   bit[10]     Send Data Regif (write start pulse)
    //   bit[11]     Read Data Regif (read start pulse)
    AVR_DAC1_DDR3_ADAPTER_CONTROL          = 0x18, // RW

    // Status Register
    //   bit[0]      DDR Initialization Complete (RO)
    //   bit[1]      Write Done Flag             (RO)
    //   bit[2]      Read Done Flag              (RO)
    AVR_DAC1_DDR3_ADAPTER_STATUS           = 0x1C  // RO
} AVR_DAC1_DDR3_Adapter_Reg;

// LVDS Interface Register Offsets
typedef enum {
    // Control Register
    //   bit 0     : Training Enable (RW, 1 = training pattern transmitting)
    //   bits[2:1] : DAC Select (RW)
    //               00 = DAC0 data
    //               01 = DAC2 data
    AVR_LVDS_CTRL               = 0x00, // RW

    // Training Pattern Registers
    //   AVR_LVDS_TRAIN_PATTERN0[31:0]   at 0x04
    //   AVR_LVDS_TRAIN_PATTERN1[63:32]  at 0x08
    //   AVR_LVDS_TRAIN_PATTERN2[95:64]  at 0x0C
    //   AVR_LVDS_TRAIN_PATTERN3[127:96] at 0x10
    //   AVR_LVDS_TRAIN_PATTERN4[135:128]at 0x14 (bits[7:0])
    AVR_LVDS_TRAIN_PATTERN0     = 0x04, // RW, bits[31:0]
    AVR_LVDS_TRAIN_PATTERN1     = 0x08, // RW, bits[31:0]
    AVR_LVDS_TRAIN_PATTERN2     = 0x0C, // RW, bits[31:0]
    AVR_LVDS_TRAIN_PATTERN3     = 0x10, // RW, bits[31:0]
    AVR_LVDS_TRAIN_PATTERN4     = 0x14  // RW, bits[7:0]
} AVR_LVDS_Interface_Reg;

// SPI Flash Interface Register Offsets
typedef enum {
    // Instruction Register (RW)
    //   bit 0 : Read Instruction (pulse)
    //   bit 1 : Write Instruction (pulse)
    //   bit 2 : Sector Erase Instruction (pulse)
    //   bit 3 : Bulk Erase Instruction (pulse)
    //   bit 4 : Reset Instruction (pulse)
    AVR_SPI_FLASH_INSTR        = 0x00,

    // Start Address Register (RW)
    //   bits[23:0] : start address for read/write/erase
    AVR_SPI_FLASH_START_ADDR   = 0x04,

    // Data Input Register (RW)
    //   bits[7:0] : data byte to write into flash
    AVR_SPI_FLASH_DATA_IN      = 0x08,

    // File Size Register (RW)
    //   bits[9:0] : number of 256-byte pages to read/write
    AVR_SPI_FLASH_FILE_SIZE    = 0x0C,

    // Status Register (RO)
    //   bit 0     : Busy (1 = flash busy)
    //   bit 1     : Almost Full FIFO (user write FIFO)
    //   bit 2     : Empty FIFO (user read FIFO)
    //   bits[15:3]: FIFO Write Level (user write FIFO depth)
    AVR_SPI_FLASH_STATUS       = 0x10,

    // Data Out Register (RO)
    //   bits[7:0] : data byte read from flash
    AVR_SPI_FLASH_DATA_OUT     = 0x14
} AVR_SPI_FLASH_Reg;

// Manufacturing Control & Status Register Offsets
typedef enum {
    // USB GPIF II Manufacturing Test Pass/Fail Status Register
    //   bit 0     : Test Pass Status (RO, 1 = pass)
    //   bits[31:1]: Reserved
    AVR_MANUF_USB_GPIF2_TEST_STATUS    = 0x00,

    // PCIe Link Status Register
    //   bit 0     : Link Up Status (RO, 1 = link up)
    //   bits[2:1] : Link Width (RO)
    //                00 = x1
    //                01 = x2
    //                10 = x4
    //   bit 3     : Link Speed (RO, 0 = Gen1, 1 = Gen2)
    //   bits[31:4]: Reserved
    AVR_MANUF_PCIE_LINK_STATUS         = 0x04,

    // Trigger I/O Control Register
    //   bits[3:0] : Trigger Out GPIO Control (RW)
    //   bits[7:4] : Trigger In GPIO Status (RO)
    //   bits[31:8]: Reserved
    AVR_MANUF_TRIGGER_IO_CTRL          = 0x08,

    // FPGA Voltage and Temperature Alarms Register
    //   bit 0     : Temperature Sensor Alarm       (RO)
    //   bit 1     : Vccint Sensor Alarm           (RO)
    //   bit 2     : Vccaux Sensor Alarm           (RO)
    //   bit 3     : Vccbram Sensor Alarm          (RO)
    //   bit 4     : Vccpint Sensor Alarm          (RO)
    //   bit 5     : Vccpaux Sensor Alarm          (RO)
    //   bit 6     : Vcco_ddr Sensor Alarm         (RO)
    //   bit 7     : Logical OR of Alarms [6:0]    (RO)
    //   bits[31:8]: Reserved
    AVR_MANUF_FPGA_VOLT_TEMP_ALARMS    = 0x0C
} AVR_Manufacturing_Ctrl_Status_Reg;

// I2C Slave Control Register Offsets
typedef enum {
    // Message Control Register
    //   bit  0   : I2C Write Data Valid (RW) – pulse 1→0 indicates data valid on MSG_DIN
    //   bit  1   : MSG_DOUT Read Enable    (RW)
    //   bit  2   : I2C_NO_STOP             (RW)
    //   bit  3   : I2C_10_BIT_ADDR         (RW)
    //   bits[11:4]: I2C Message Size       (RW) – number of bytes per I2C message
    //   bits[19:12]: I2C Message Code      (RW)
    //       bit  19: reserved
    //       bit  18: ‘0’ = 1-byte register offset; ‘1’ = 2-byte register offset
    //       bits[17:16]: 00=send; 01=reset; 10=stop; 11=reserved
    //       bits[15:12]: 0→write; 1→two-part read; 2→combined read; 3→short read; 4–F=reserved
    //   bit  20  : I2C Message Valid       (RW)
    //   bits[24:21]: I2C_SPEED             (RW)
    //   bits[30:25]: CLOCK_FREQKHZ_reg     (RW)
    AVR_I2C_MSG_CTRL          = 0x00,

    // Slave Address & Register Address
    //   bits[31:16]: I2C Slave Address      (RW)
    //   bits[15: 0]: I2C Slave Register Addr (RW)
    AVR_I2C_SLAVE_ADDR        = 0x04,

    // Write Data Register
    //   bits[7:0]: I2C Write Data            (RW)
    AVR_I2C_WRITE_DATA        = 0x08,

    // Status Register
    //   bit  0   : I2C Busy                  (RO)
    //   bit  1   : FIFO Full                 (RO)
    //   bit  2   : I2C Read Data Valid       (RO)
    //   bits[10:3]: I2C Command Status       (RO)
    //       3 = STATUS_OK; 4 = BUS_ERROR; 5 = SLA_ACK;
    //       6 = SLA_NACK; 7 = DATA_NACK; 8 = ARB_LOST;
    //       9 = BUS_LOCKED; 10 = LAST_DATA_ACK
    //   bits[19:11]: Byte Count              (RO) – bytes written/read before NACK/stop
    //   bit  20  : I2C Message Done           (RO)
    //   bits[25:21]: Debug State              (RO)
    AVR_I2C_STATUS            = 0x0C,

    // Read Data Register
    //   bits[7:0]: I2C Read Data             (RO)
    AVR_I2C_READ_DATA         = 0x10

} AVR_I2C_Slave_Control_Reg;

// Interface Registers Offsets
typedef enum {
    // DDR3 Memory Access Register
    //   bits[31:0]: register address for bulk data transfer (common for all SW interfaces)
    AVR_INTERFACE_DDR3_MEM_ACCESS     = 0x00, // RW

    // SPI Flash Memory Access Register
    //   bits[31:0]: register address for bulk data transfer (common for all SW interfaces)
    AVR_INTERFACE_SPI_FLASH_MEM_ACCESS = 0x04, // RW

    // Interface Selection Register
    //   bits[1:0]: selects active interface
    //       00 = Ethernet Interface
    //       01 = UART Interface
    //       10 = USB Interface
    //       11 = PCIe Interface
    //   bits[31:2]: reserved
    AVR_INTERFACE_SELECTION           = 0x08  // RW
} AVR_Interface_Reg;

// Clock Reset Control Register Offsets
typedef enum {
    // Reset Registers & MMCME2 DRP Registers (RW, pulse)
    //   bit  0 : DAC# SPI Interface Reset
    //   bit  1 : LVDS Reset
    //   bit  2 : DAC Interface Reset
    //   bit  3 : DDR3 IF Reset
    //   bit  4 : AXI Lite Reset
    //   bit  5 : Design PLL Reset
    //   bit  6 : DAC PLL Reset
    //   bit  7 : Reset CLK Counter
    //   bit  8 : Start CLK Counter
    //   bit  9 : Design Reset
    //   bit 10 : DDR3 Test Reset
    //   bit 11 : SPI FLASH Reset
    //   bit 12 : USB Interface Reset
    //   bit 13 : UART Interface Reset
    //   bit 14 : Ethernet Interface Reset
    //   bit 15 : SSTEP (DRP trigger pulse)
    //   bits[17:16]: STATE (MMCME2 DRP interpolation factor)
    //                 01 = x2, 10 = x4, 11 = x8
    //   bit 18 : PCIe Interface Reset
    //   bit 21 : SD Card Interface Reset
    //   bit 22 : I2C Interface Reset
    AVR_CLK_RST_CTRL_RESET_REGS        = 0x00,

    // Timing Window Counter (RW, bits[31:0])
    AVR_CLK_RST_CTRL_TIMING_WINDOW     = 0x04,

    // FPGA OUT Clock Counter (RO, bits[31:0])
    AVR_CLK_RST_CTRL_FPGA_OUT_CLK      = 0x08,

    // DAC DATA Clock Counter (RO, bits[31:0])
    AVR_CLK_RST_CTRL_DAC_DATA_CLK      = 0x0C,

    // PLL Lock Status Register (RO)
    //   bit  0 : DAC PLL Locked
    //   bit  1 : Design PLL Locked
    AVR_CLK_RST_CTRL_PLL_LOCK_STATUS   = 0x10,

    // Temperature Limit Register (RW, bits[6:0])
    //   Temperature in °C above which alarm is set
    AVR_CLK_RST_CTRL_TEMP_LIMIT        = 0x14,

    // General Purpose Software Register (RW, bits[31:0])
    AVR_CLK_RST_CTRL_GP_SOFT_REG       = 0x18,

    // Temperature Value / Alarm / SD Card Boot Register (RO)
    //   bits[15:0]: Temp Value
    //       bit 15 : Sign
    //       bits[14:8] : Integer part
    //       bits[7:4]  : Decimal part
    //       bits[3:0]  : Reserved
    //   bit 16 : Temp Alarm
    //   bit 17 : SD Card Boot (SW3 position)
    AVR_CLK_RST_CTRL_TEMP_VALUE        = 0x1C
} AVR_CLK_RST_CTRL_Reg;

/* STEP 1: all supported interfaces */
typedef enum {
    AVR_IFACE_SPI_CTRL = 0,
    AVR_IFACE_DDR3_TDG,
    AVR_IFACE_DDR3_RW_ADAPTER,
    AVR_IFACE_DAC_IF,
    AVR_IFACE_DAC0_ADAPTER,
    AVR_IFACE_DAC1_ADAPTER,
    AVR_IFACE_LVDS_IF,
    AVR_IFACE_SPI_FLASH,
    AVR_IFACE_MANUF_CTRL_STATUS,
    AVR_IFACE_I2C_SLAVE_CTRL,
    AVR_IFACE_COMMON_INTERFACE,
    AVR_IFACE_CLK_RST_CTRL,
    AVR_IFACE_COUNT
} AVR_InterfaceType;

// /* STEP 2: base‐address table, ordered by AVR_InterfaceType */
// static const uint32_t g_avrBaseAddrs[AVR_IFACE_COUNT] = {
//     [AVR_IFACE_SPI_CTRL]            = AVR_SPI_CTRL_BASE_ADDR,
//     [AVR_IFACE_DDR3_TDG]            = AVR_DDR3_TDG_BASE_ADDR,
//     [AVR_IFACE_DDR3_RW_ADAPTER]     = AVR_DDR3_RW_ADAPTER_BASE_ADDR,
//     [AVR_IFACE_DAC_IF]              = AVR_DAC_INTERFACE_BASE_ADDR,
//     [AVR_IFACE_DAC0_ADAPTER]        = AVR_DAC0_DDR3_ADAPTER_BASE_ADDR,
//     [AVR_IFACE_DAC1_ADAPTER]        = AVR_DAC1_DDR3_ADAPTER_BASE_ADDR,
//     [AVR_IFACE_LVDS_IF]             = AVR_LVDS_INTERFACE_BASE_ADDR,
//     //[AVR_IFACE_SPI_FLASH]           = AVR_SPI_FLASH_BASE_ADDR,
//     [AVR_IFACE_MANUF_CTRL_STATUS]   = AVR_MANUF_CTRL_STATUS_BASE_ADDR,
//     [AVR_IFACE_I2C_SLAVE_CTRL]      = AVR_I2C_SLAVE_CTRL_BASE_ADDR,
//     [AVR_IFACE_COMMON_INTERFACE]    = AVR_INTERFACE_BASE_ADDR,
//     [AVR_IFACE_CLK_RST_CTRL]        = AVR_CLK_RST_CTRL_BASE_ADDR
// };

#endif // AVR_SPI_CONTROL_REG_H

