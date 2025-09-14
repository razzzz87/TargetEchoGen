#pragma once
#include <cstdint>
#define BASE_ADDR_WB 0x00001000
#define BASE_ADDR_NB 0x00001200


enum eWB_Offset{
    eWB_ADC_CH_SEL             = 0x0000, // 0x1000: Bits 0–3: ADC ch01–ch04, Bit 31: all ADC
    eWB_DDS_EN                 = 0x0004, // 0x1004: Bit 0: DDS enable, Bit 1: DDS set
    eWB_DDS_FCW                = 0x0008, // 0x1008: Bit 28: DDC FCW
    eWB_CIC_EN                 = 0x000C, // 0x100C: Bit 0: CIC enable
    eWB_CIC_DECIMATION         = 0x0010, // 0x1010: Bit 14: CIC decimation
    eWB_CIC_DECIMATION_VALID   = 0x0014, // 0x1014: Bit 0: CIC decimation valid
    eWB_CIC_FS_IN              = 0x0018, // 0x1018: Bit 20: CIC Fs in
    eWB_CIC_FS_OUT             = 0x001C, // 0x101C: Bit 16: CIC Fs out
    eWB_CFIR_EN                = 0x0020, // 0x1020: Bit 0: CFIR enable
    eWB_CFIR_DECIMATION        = 0x0024, // 0x1024: Bit 14: CFIR decimation
    eWB_CFIR_DECIMATION_VALID  = 0x0028, // 0x1028: Bit 0: CFIR decimation valid
    eWB_CFIR_FS_IN             = 0x002C, // 0x102C: Bit 17: CFIR Fs in
    eWB_CFIR_FS_OUT            = 0x0030, // 0x1030: Bit 17: CFIR Fs out
    eWB_FFT_WIN_EN             = 0x0034, // 0x1034: Bit 0: FFT window enable
    eWB_FFT_WIN_FS             = 0x0038, // 0x1038: Bit 13: FFT window Fs
    eWB_FFT_WIN_SIZE           = 0x003C, // 0x103C: Bit 16: FFT window size
    eWB_FFT_CONFIG_EN          = 0x0040, // 0x1040: Bit 0: FFT config enable
    eWB_FFT_CONFIG_FS          = 0x0044, // 0x1044: Bit 18: FFT config Fs
    eWB_FFT_CONFIG_SIZE        = 0x0048  // 0x1048: Bit 15: FFT config size
};

enum eWB_RegVal{
    eWB_ADC_CH_SEL_VAL             = 0x1F,  // ADC ch01–ch04 + all ADC
    eWB_DDS_CTRL_VAL               = 0x03,  // DDS enable + set
    eWB_DDS_EN_VAL                 = 0x01,
    eWB_DDC_FCW_VAL                = 0x16,  // Bit 28
    eWB_CIC_EN_VAL                 = 0x01,  // Bit 0
    eWB_CIC_DECIMATION_VAL         = 0x12,  // Bit 14
    eWB_CIC_DECIMATION_VALID_VAL   = 0x01,  // Bit 0
    eWB_CIC_FS_IN_VAL              = 0x20,  // Bit 20
    eWB_CIC_FS_OUT_VAL             = 0x16,  // Bit 16
    eWB_CFIR_EN_VAL                = 0x01,  // Bit 0
    eWB_CFIR_DECIMATION_VAL        = 0x14,  // Bit 14
    eWB_CFIR_DECIMATION_VALID_VAL  = 0x01,  // Bit 0
    eWB_CFIR_FS_IN_VAL             = 0x17,  // Bit 17
    eWB_CFIR_FS_OUT_VAL            = 0x17,  // Bit 17
    eWB_FFT_WIN_EN_VAL             = 0x01,  // Bit 0
    eWB_FFT_WIN_FS_VAL             = 0x13,  // Bit 13
    eWB_FFT_WIN_SIZE_VAL           = 0x16,  // Bit 16
    eWB_FFT_CONFIG_EN_VAL          = 0x01,  // Bit 0
    eWB_FFT_CONFIG_FS_VAL          = 0x18,  // Bit 18
    eWB_FFT_CONFIG_SIZE_VAL        = 0x15   // Bit 15
};

//enum RegAddrOffset {
//    eWB_DDC_ENBLE = 0x01
//};

// enum WBRegAddrOffset {
//     eWB_ALL_ADC_CHNL_SEL = 0x00,
//     eWB_DDS_SET = 0x04,
//     eWB_DDS_FCW = 0x08,
//     eWB_CIC_EN = 0x0C,
//     eWB_CIC_DECIMATION = 0x10,
//     eWB_CIC_DECIMATION_VALID = 0x14,
//     eWB_CIC_FS_IN = 0X18,
//     eWB_CIC_FS_OUT = 0X1C,
//     eWB_CFIR_EN = 0X20,
//     eWB_CFIR_DECIMATION = 0X24,
//     eWB_CFIR_DECIMATION_valid = 0X28,
//     eWB_CFIR_FS_IN = 0X2C,
//     eWB_CFIR_FS_OUT = 0X30,
//     eWB_FFT_WIN_EN = 0X34,
//     eWB_FFT_WIN_FS = 0X38,
//     eWB_FFT_WIN_CONFIG_EN = 0X40,
//     eWB_FFT_WIN_CONFIG_FS = 0X44,
//     eWB_FFT_WIN_CONFIG_SIZE = 0X48,
//     eWB_FFT_WIN_SIZE_ = 0X3C,
// };

// enum WBRegVal {
//     eWB_ALL_ADC_CHNL_SEL_VAL = 0x1F,
//     eWB_DDS_SET_VAL = 0x3,
//     eWB_DDS_FCW_VAL = 0x16,
//     eWB_CIC_EN_VAL = 0x01,
//     eWB_CIC_DECIMATION_VAL = 0x12,
//     eWB_CIC_DECIMATION_VALID_VAL = 0x01,
//     eWB_CIC_FS_IN_VAL = 0X20,
//     eWB_CIC_FS_OUT_VAL = 0X16,
//     eWB_CFIR_EN_VAL = 0X10,
//     eWB_CFIR_DECIMATION_VAL = 0X14,
//     eWB_CFIR_DECIMATION_valid_val = 0X01,
//     eWB_CFIR_FS_IN_VAL= 0X17,
//     eWB_CFIR_FS_OUT_VAL = 0X17,
//     eWB_FFT_WIN_EN_VAL = 0X01,
//     eWB_FFT_WIN_FS_VAL = 0X13,
//     eWB_FFT_WIN_CONFIG_EN_VAL = 0X01,
//     eWB_FFT_WIN_CONFIG_FS_VAL = 0X18,
//     eWB_FFT_WIN_CONFIG_SIZE_VAL = 0X15,
//     eWB_FFT_WIN_SIZE_VAL = 0X16
// };



enum eNB_Offset{
    eNB_ADC_CH_SEL             = 0x0000, // 0x1200: Bit 0–3: ADC ch01–ch04, Bit 31: all ADC
    eNB_DDS_EN                 = 0x0004, // 0x1204: Bit 0: DDS enable, Bit 1: DDS set
    eNB_DDC_FCW                = 0x0008, // 0x1208: Bit 16: DDC FCW
    eNB_CIC_EN                 = 0x000C, // 0x120C: Bit 0: CIC enable
    eNB_CIC_DECIMATION         = 0x0010, // 0x1210: Bit 12: CIC decimation
    eNB_CIC_DECIMATION_VALID   = 0x0014, // 0x1214: Bit 0: CIC decimation valid
    eNB_CIC_FS_IN              = 0x0018, // 0x1218: Bit 0: CIC Fs in
    eNB_CIC_FS_OUT             = 0x001C, // 0x121C: Bit 16: CIC Fs out
    eNB_PFIR_EN                = 0x0020, // 0x1220: Bit 0: PFIR enable
    eNB_PFIR_DECIMATION        = 0x0024, // 0x1224: Bit 14: PFIR decimation
    eNB_PFIR_DECIMATION_VALID  = 0x0028, // 0x1228: Bit 0: PFIR decimation valid
    eNB_PFIR_FS_IN             = 0x002C, // 0x122C: Bit 17: PFIR Fs in
    eNB_PFIR_FS_OUT            = 0x0030, // 0x1230: Bit 17: PFIR Fs out
    eNB_CFIR_EN                = 0x0034, // 0x1234: Bit 0: CFIR enable
    eNB_CFIR_DECIMATION        = 0x0038, // 0x1238: Bit 13: CFIR decimation
    eNB_CFIR_DECIMATION_VALID  = 0x003C, // 0x123C: Bit 0: CFIR decimation valid
    eNB_CFIR_FS_IN             = 0x0040, // 0x1240: Bit 20: CFIR Fs in
    eNB_CFIR_FS_OUT            = 0x0044, // 0x1244: Bit 18: CFIR Fs out
    eNB_FFT_WIN_EN             = 0x0048, // 0x1248: Bit 0: FFT window enable
    eNB_FFT_WIN_FS             = 0x004C, // 0x124C: Bit 16: FFT window Fs
    eNB_FFT_WIN_SIZE           = 0x0050, // 0x1250: Bit 19: FFT window size
    eNB_FFT_CONFIG_EN          = 0x0054, // 0x1254: Bit 0: FFT config enable
    eNB_FFT_CONFIG_FS          = 0x0058, // 0x1258: Bit 16: FFT config Fs
    eNB_FFT_CONFIG_SIZE        = 0x005C, // 0x125C: Bit 16: FFT config size
    eNB_DDC_CH01_32_SEL        = 0x0060, // 0x1260: Bit 16: DDC ch01–32 select
    eNB_DDC_CH33_64_SEL        = 0x0064, // 0x1264: Bit 16: DDC ch33–64 select
    eNB_DDC_ALL_CH_CTRL        = 0x0068  // 0x1268: Bits 0–4: DDC enable, windowing, FFT enable
};

enum eNB_RegVal {
    eNB_ADC_CH_SEL_VAL             = 0x1F,  // Bits 0–3: ADC channels, Bit 31: all ADC
    eNB_DDS_EN_VAL                 = 0x03,  // Bits 0–1: DDS enable/set
    eNB_DDC_FCW_VAL                = 0x16,  // Bit 16
    eNB_CIC_EN_VAL                 = 0x01,  // Bit 0
    eNB_CIC_DECIMATION_VAL         = 0x12,  // Bit 12
    eNB_CIC_DECIMATION_VALID_VAL   = 0x01,  // Bit 0
    eNB_CIC_FS_IN_VAL              = 0x20,  // Bit 0
    eNB_CIC_FS_OUT_VAL             = 0x16,  // Bit 16
    eNB_PFIR_EN_VAL                = 0x01,  // Bit 0
    eNB_PFIR_DECIMATION_VAL        = 0x14,  // Bit 14
    eNB_PFIR_DECIMATION_VALID_VAL  = 0x01,  // Bit 0
    eNB_PFIR_FS_IN_VAL             = 0x17,  // Bit 17
    eNB_PFIR_FS_OUT_VAL            = 0x17,  // Bit 17
    eNB_CFIR_EN_VAL                = 0x01,  // Bit 0
    eNB_CFIR_DECIMATION_VAL        = 0x13,  // Bit 13
    eNB_CFIR_DECIMATION_VALID_VAL  = 0x01,  // Bit 0
    eNB_CFIR_FS_IN_VAL             = 0x20,  // Bit 20
    eNB_FFT_WIN_EN_NB_VAL          = 0x18,  // Bit 18
    eNB_FFT_WIN_EN_VAL             = 0x01,  // Bit 0
    eNB_FFT_WIN_FS_VAL             = 0x16,  // Bit 16
    eNB_FFT_WIN_SIZE_VAL           = 0x19,  // Bit 19
    eNB_FFT_CONFIG_EN_VAL          = 0x01,  // Bit 0
    eNB_FFT_CONFIG_FS_VAL          = 0x16,  // Bit 16
    eNB_FFT_CONFIG_SIZE_VAL        = 0x16,  // Bit 16
    eNB_DDC_CH01_32_SEL_VAL        = 0x16,  // Bit 16
    eNB_DDC_CH33_64_SEL_VAL        = 0x16,  // Bit 16
    eNB_DDC_ALL_CH_CTRL_VAL        = 0x2F   // Bits 0–4: DDC, windowing, FFT enable
};

enum WBChannel {
    eWB_CHNL1 = 0,
    eWB_CHNL2 = 1,
    eWB_CHNL3 = 2,
    eWB_CHNL4 = 3
 };

enum NBChannel{
    eNB_CHNL1  = 0,
    eNB_CHNL2  = 1,
    eNB_CHNL3  = 2,
    eNB_CHNL4  = 3,
    eNB_CHNL5  = 4,
    eNB_CHNL6  = 5,
    eNB_CHNL7  = 6,
    eNB_CHNL8  = 7,
    eNB_CHNL9  = 8,
    eNB_CHNL10 = 9,
    eNB_CHNL11 = 10,
    eNB_CHNL12 = 11,
    eNB_CHNL13 = 12,
    eNB_CHNL14 = 13,
    eNB_CHNL15 = 14,
    eNB_CHNL16 = 15,
    eNB_CHNL17 = 16,
    eNB_CHNL18 = 17,
    eNB_CHNL19 = 18,
    eNB_CHNL20 = 19,
    eNB_CHNL21 = 20,
    eNB_CHNL22 = 21,
    eNB_CHNL23 = 22,
    eNB_CHNL24 = 23,
    eNB_CHNL25 = 24,
    eNB_CHNL26 = 25,
    eNB_CHNL27 = 26,
    eNB_CHNL28 = 27,
    eNB_CHNL29 = 28,
    eNB_CHNL30 = 29,
    eNB_CHNL31 = 30,
    eNB_CHNL32 = 31,
    eNB_CHNL33 = 32,
    eNB_CHNL34 = 33,
    eNB_CHNL35 = 34,
    eNB_CHNL36 = 35,
    eNB_CHNL37 = 36,
    eNB_CHNL38 = 37,
    eNB_CHNL39 = 38,
    eNB_CHNL40 = 39,
    eNB_CHNL41 = 40,
    eNB_CHNL42 = 41,
    eNB_CHNL43 = 42,
    eNB_CHNL44 = 43,
    eNB_CHNL45 = 44,
    eNB_CHNL46 = 45,
    eNB_CHNL47 = 46,
    eNB_CHNL48 = 47,
    eNB_CHNL49 = 48,
    eNB_CHNL50 = 49,
    eNB_CHNL51 = 50,
    eNB_CHNL52 = 51,
    eNB_CHNL53 = 52,
    eNB_CHNL54 = 53,
    eNB_CHNL55 = 54,
    eNB_CHNL56 = 55,
    eNB_CHNL57 = 56,
    eNB_CHNL58 = 57,
    eNB_CHNL59 = 58,
    eNB_CHNL60 = 59,
    eNB_CHNL61 = 60,
    eNB_CHNL62 = 61,
    eNB_CHNL63 = 62,
    eNB_CHNL64 = 63
};

