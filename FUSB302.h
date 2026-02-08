#ifndef FUSB302_H
#define FUSB302_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// #define FUSB302_DEBUG

// I2C address
#define FUSB302_I2C_ADDR 0x22

// Register addresses
#define FUSB302_REG_ALL (-1)

// Control registers
#define FUSB302_REG_CONTROL_START 0x01
#define FUSB302_REG_CONTROL_NUM 16

#define FUSB302_REG_DEVICE_ID 0x01
#define FUSB302_REG_SWITCHES0 0x02
#define FUSB302_REG_SWITCHES1 0x03
#define FUSB302_REG_MEASURE 0x04
#define FUSB302_REG_SLICE 0x05
#define FUSB302_REG_CONTROL0 0x06
#define FUSB302_REG_CONTROL1 0x07
#define FUSB302_REG_CONTROL2 0x08
#define FUSB302_REG_CONTROL3 0x09
#define FUSB302_REG_MASK 0x0A
#define FUSB302_REG_POWER 0x0B
#define FUSB302_REG_RESET 0x0C
#define FUSB302_REG_OCREG 0x0D
#define FUSB302_REG_MASKA 0x0E
#define FUSB302_REG_MASKB 0x0F
#define FUSB302_REG_CONTROL4 0x10

// Status registers
#define FUSB302_REG_STATUS_START 0x3C
#define FUSB302_REG_STATUS_NUM 7

#define FUSB302_REG_STATUS0A 0x3C
#define FUSB302_REG_STATUS1A 0x3D
#define FUSB302_REG_INTERRUPTA 0x3E
#define FUSB302_REG_INTERRUPTB 0x3F
#define FUSB302_REG_STATUS0 0x40
#define FUSB302_REG_STATUS1 0x41
#define FUSB302_REG_INTERRUPT 0x42

#define FUSB302_REG_FIFOS 0x43

// FUSB302_REG_DEVICE_ID, all R
#define FUSB302_VERSION_ID_BITS 0xF0
#define FUSB302_VERSION_ID_OFFSET 4

#define FUSB302_REVISION_ID_BITS 0x0F
#define FUSB302_REVISION_ID_OFFSET 0

// FUSB302_REG_SWITCHES0, all R/W
#define FUSB302_PU_EN2 (1 << 7)
#define FUSB302_PU_EN1 (1 << 6)
#define FUSB302_VCONN_CC2 (1 << 5)
#define FUSB302_VCONN_CC1 (1 << 4)
#define FUSB302_MEAS_CC2 (1 << 3)
#define FUSB302_MEAS_CC1 (1 << 2)
#define FUSB302_PDWN2 (1 << 1)
#define FUSB302_PDWN1 (1 << 0)

// FUSB302_REG_SWITCHES1, all R/W
#define FUSB302_POWERROLE (1 << 7)

#define FUSB302_SPECREV_BITS (0x3 << 5)
#define FUSB302_SPECREV_OFFSET 5

#define FUSB302_DATAROLE (1 << 4)
#define FUSB302_AUTOCRC (1 << 2)
#define FUSB302_TXCC2 (1 << 1)
#define FUSB302_TXCC1 (1 << 0)

// FUSB302_REG_MEASURE, all R/W
#define FUSB302_MEAS_VBUS (1 << 6)

#define FUSB302_MDAC_BITS (0x3F)
#define FUSB302_MDAC_OFFSET 0
#define FUSB302_MDAC_LSB_MV 42
#define FUSB302_MDAC_ZERO_MV 42

// FUSB302_REG_SLICE, all R/W
#define FUSB302_SDAC_HYS_BITS (0x3 << 6)
#define FUSB302_SDAC_HYS_OFFSET 6
#define FUSB302_SDAC_HYS_255MV 0x3
#define FUSB302_SDAC_HYS_170MV 0x2
#define FUSB302_SDAC_HYS_85MV 0x1
#define FUSB302_SDAC_HYS_NONE 0x0

#define FUSB302_SDAC_BITS (0x3F << 0)
#define FUSB302_SDAC_OFFSET 0

// FUSB302_REG_CONTROL0, all R/W except noted
#define FUSB302_TX_FLUSH (1 << 6) // W/C
#define FUSB302_INT_MASK (1 << 5)

#define FUSB302_HOST_CUR_BITS (0x3 << 2)
#define FUSB302_HOST_CUR_OFFSET 2
#define FUSB302_HOST_CUR_NONE 0x0
#define FUSB302_HOST_CUR_DEF_USB 0x1
#define FUSB302_HOST_CUR_1_5A 0x2
#define FUSB302_HOST_CUR_3A 0x3

#define FUSB302_AUTO_PRE (1 << 1)
#define FUSB302_TX_START (1 << 0) // W/C

// FUSB302_REG_CONTROL1, all R/W except noted
#define FUSB302_ENSOP2DB (1 << 6)
#define FUSB302_ENSOP1DB (1 << 5)
#define FUSB302_BIST_MODE2 (1 << 4)
#define FUSB302_RX_FLUSH (1 << 2) // W/C
#define FUSB302_ENSOP2 (1 << 1)
#define FUSB302_ENSOP1 (1 << 0)

// FUSB302_REG_CONTROL2, all R/W
#define FUSB302_TOG_SAVE_PWR_BITS (0x3 << 6)
#define FUSB302_TOG_SAVE_PWR_OFFSET 6
#define FUSB302_TOG_SAVE_PWR_NONE 0x0
#define FUSB302_TOG_SAVE_PWR_40MS 0x1
#define FUSB302_TOG_SAVE_PWR_80MS 0x2
#define FUSB302_TOG_SAVE_PWR_160MS 0x3

#define FUSB302_TOG_RD_ONLY (1 << 5)
#define FUSB302_WAKE_EN (1 << 3)

#define FUSB302_MODE_BITS (0x3 << 1)
#define FUSB302_MODE_OFFSET 1
#define FUSB302_MODE_DO_NOT_USE 0x0
#define FUSB302_MODE_TOGGLE_DRP 0x1
#define FUSB302_MODE_TOGGLE_SNK 0x2
#define FUSB302_MODE_TOGGLE_SRC 0x3

#define FUSB302_TOGGLE (1 << 0)

// FUSB302_REG_CONTROL3, all R/W except noted
#define FUSB302_SEND_HARD_RESET (1 << 6) // W/C
#define FUSB302_AUTO_HARDRESET (1 << 4)
#define FUSB302_AUTO_SOFTRESET (1 << 3)

#define FUSB302_N_RETRIES_BITS (0x3 << 1)
#define FUSB302_N_RETRIES_OFFSET 1
#define FUSB302_N_RETRIES_NONE 0x0
#define FUSB302_N_RETRIES_1 0x1
#define FUSB302_N_RETRIES_2 0x2
#define FUSB302_N_RETRIES_3 0x3

#define FUSB302_AUTO_RETRY (1 << 0)

// FUSB302_REG_MASK, all R/W
#define FUSB302_M_VBUSOK (1 << 7)
#define FUSB302_M_ACTIVITY (1 << 6)
#define FUSB302_M_COMP_CHNG (1 << 5)
#define FUSB302_M_CRC_CHK (1 << 4)
#define FUSB302_M_ALERT (1 << 3)
#define FUSB302_M_WAKE (1 << 2)
#define FUSB302_M_COLLISION (1 << 1)
#define FUSB302_M_BC_LVL (1 << 0)

// FUSB302_REG_POWER, all R/W
#define FUSB302_PWR_INT_OSC (1 << 3)
#define FUSB302_PWR_MEAS_BLOCK (1 << 2)
#define FUSB302_PWR_RECV_CUR (1 << 1)
#define FUSB302_PWR_BANDGAP_WAKE (1 << 0)

// FUSB302_REG_RESET, all W/C
#define FUSB302_PD_RESET (1 << 1)
#define FUSB302_SW_RESET (1 << 0)

// FUSB302_REG_OCPREG, all R/W
#define FUSB302_OCP_RANGE (1 << 3)

#define FUSB302_OCP_CUR_BITS (0x7)
#define FUSB302_OCP_CUR_OFFSET 0
#define FUSB302_OCP_CUR_MAX_RANGE (0b111)
#define FUSB302_OCP_CUR_7_8_MAX_RANGE (0b110)
#define FUSB302_OCP_CUR_6_8_MAX_RANGE (0b101)
#define FUSB302_OCP_CUR_5_8_MAX_RANGE (0b100)
#define FUSB302_OCP_CUR_4_8_MAX_RANGE (0b011)
#define FUSB302_OCP_CUR_3_8_MAX_RANGE (0b010)
#define FUSB302_OCP_CUR_2_8_MAX_RANGE (0b001)
#define FUSB302_OCP_CUR_1_8_MAX_RANGE (0b000)

// FUSB302_REG_MASKA, all R/W
#define FUSB302_M_OCP_TEMP (1 << 7)
#define FUSB302_M_TOGDONE (1 << 6)
#define FUSB302_M_SOFTFAIL (1 << 5)
#define FUSB302_M_RETRYFAIL (1 << 4)
#define FUSB302_M_HARDSENT (1 << 3)
#define FUSB302_M_TXSENT (1 << 2)
#define FUSB302_M_SOFTRST (1 << 1)
#define FUSB302_M_HARDRST (1 << 0)

// FUSB302_REG_MASKB, all R/W
#define FUSB302_M_GCRCSENT (1 << 0)

// FUSB302_REG_CONTROL4, all R/W
#define FUSB302_TOG_USRC_EXIT (1 << 0)

// FUSB302_STATUS0A, all R
#define FUSB302_SOFTFAIL (1 << 5)
#define FUSB302_RETRYFAIL (1 << 4)
#define FUSB302_POWER3 (1 << 3)
#define FUSB302_POWER2 (1 << 2)
#define FUSB302_SOFTRST (1 << 1)
#define FUSB302_HARDRST (1 << 0)

// FUSB302_STATUS1A, all R
#define FUSB302_TOGSS_BITS (0x7 << 3)
#define FUSB302_TOGSS_OFFSET 3
#define FUSB302_TOGSS_RUNNING 0b000
#define FUSB302_TOGSS_STOP_SRC1 0b001
#define FUSB302_TOGSS_STOP_SRC2 0b010
#define FUSB302_TOGSS_STOP_SNK1 0b101
#define FUSB302_TOGSS_STOP_SNK2 0b110
#define FUSB302_TOGSS_AUDIO 0b111

#define FUSB302_RXSOP2DB (1 << 2)
#define FUSB302_RXSOP1DB (1 << 1)
#define FUSB302_RXSOP (1 << 0)

// FUSB302_REG_INTERRUPTA, all R/C
#define FUSB302_I_OCP_TEMP (1 << 7)
#define FUSB302_I_TOGDONE (1 << 6)
#define FUSB302_I_SOFTFAIL (1 << 5)
#define FUSB302_I_RETRYFAIL (1 << 4)
#define FUSB302_I_HARDSENT (1 << 3)
#define FUSB302_I_TXSENT (1 << 2)
#define FUSB302_I_SOFTRST (1 << 1)
#define FUSB302_I_HARDRST (1 << 0)

// FUSB302_REG_INTERRUPTB, all R/C
#define FUSB302_I_GCRCSENT (1 << 0)

// FUSB302_REG_STATUS0, all R
#define FUSB302_VBUSOK (1 << 7)
#define FUSB302_ACTIVITY (1 << 6)
#define FUSB302_COMP (1 << 5)
#define FUSB302_CRC_CHK (1 << 4)
#define FUSB302_ALERT (1 << 3)
#define FUSB302_WAKE (1 << 2)

#define FUSB302_BC_LVL_BITS (0x3)
#define FUSB302_BC_LVL_OFFSET 0
#define FUSB302_BC_LVL_0_200MV 0b00
#define FUSB302_BC_LVL_200_660MV 0b01
#define FUSB302_BC_LVL_660_1230MV 0b10
#define FUSB302_BC_LVL_1230MV_MORE 0b11

// FUSB302_REG_STATUS1, all R
#define FUSB302_RXSOP2 (1 << 7)
#define FUSB302_RXSOP1 (1 << 6)
#define FUSB302_RX_EMPTY (1 << 5)
#define FUSB302_RX_FULL (1 << 4)
#define FUSB302_TX_EMPTY (1 << 3)
#define FUSB302_TX_FULL (1 << 2)
#define FUSB302_OVRTEMP (1 << 1)
#define FUSB302_OCP (1 << 0)

// FUSB302_REG_INTERRUPT, all R/C
#define FUSB302_I_VBUSOK (1 << 7)
#define FUSB302_I_ACTIVITY (1 << 6)
#define FUSB302_I_COMP_CHNG (1 << 5)
#define FUSB302_I_CRC_CHK (1 << 4)
#define FUSB302_I_ALERT (1 << 3)
#define FUSB302_I_WAKE (1 << 2)
#define FUSB302_I_COLLISION (1 << 1)
#define FUSB302_I_BC_LVL (1 << 0)

// FUSB302_REG_FIFOS, R/W
#define FUSB302_TX_RX_TOKEN_BITS 0xFF
#define FUSB302_TX_RX_TOKEN_OFFSET 0

typedef enum FUSB302_HostCurrentMode {
    FUSB302_HOST_CURRENT_MODE_500MA,
    FUSB302_HOST_CURRENT_MODE_1_5A,
    FUSB302_HOST_CURRENT_MODE_3A
} FUSB302_HostCurrentMode_t;

typedef enum FUSB302_CC_Orientation {
    FUSB302_CC_ORIENTATION_CC1,
    FUSB302_CC_ORIENTATION_CC2,
    FUSB302_CC_ORIENTATION_UNKNOWN,
} FUSB302_CC_Orientation_t;

typedef enum FUSB302_AttachedType {
    FUSB302_ATTACHED_TYPE_NONE,
    FUSB302_ATTACHED_TYPE_DEVICE,
    FUSB302_ATTACHED_TYPE_HOST_500MA,
    FUSB302_ATTACHED_TYPE_HOST_1_5A,
    FUSB302_ATTACHED_TYPE_HOST_3A,
    FUSB302_ATTACHED_TYPE_CABLE,
    FUSB302_ATTACHED_TYPE_UNKNOWN
} FUSB302_AttachedType_t;

typedef struct FUSB302_Platform {
    int (*i2cWriteReg)(uint8_t addr7bit, uint8_t regNum, const uint8_t *data, uint8_t length,
                       uint8_t wait);
    int (*i2cReadReg)(uint8_t addr7bit, uint8_t regNum, uint8_t *data, uint8_t length, int timeout);
    void (*delayUs)(uint32_t us);
    void (*debugPrint)(const char *fmt, ...);
} FUSB302_Platform_t;

typedef struct FUSB302_Data {
    uint8_t controlRegData[FUSB302_REG_CONTROL_NUM];
    uint8_t statusRegData[FUSB302_REG_STATUS_NUM];
} FUSB302_Data_t;

bool FUSB302_ReadControlData(FUSB302_Platform_t *platform, FUSB302_Data_t *data, int reg);
bool FUSB302_WriteControlData(FUSB302_Platform_t *platform, FUSB302_Data_t *data, int reg);
void FUSB302_DebugPrintControlData(FUSB302_Platform_t *platform, FUSB302_Data_t *data, int reg);

bool FUSB302_ReadStatusData(FUSB302_Platform_t *platform, FUSB302_Data_t *data, int reg);
void FUSB302_DebugPrintStatusData(FUSB302_Platform_t *platform, FUSB302_Data_t *data, int reg);

bool FUSB302_ReadFIFO(FUSB302_Platform_t *platform, uint8_t *data, uint8_t length);
bool FUSB302_WriteFIFO(FUSB302_Platform_t *platform, uint8_t *data, uint8_t length);

uint8_t *FUSB302_GetRegPtr(FUSB302_Data_t *data, int reg);

int FUSB302_GetDataBit(FUSB302_Data_t *data, int reg, int bitMask);
void FUSB302_SetDataBit(FUSB302_Data_t *data, int reg, int bitMask, int value);

int FUSB302_GetDataValue(FUSB302_Data_t *data, int reg, int bitMask, int offset);
void FUSB302_SetDataValue(FUSB302_Data_t *data, int reg, int bitMask, int offset, int value);

bool FUSB302_Reset(FUSB302_Platform_t *platform, FUSB302_Data_t *data);

#ifdef __cplusplus
}
#endif

#endif // FUSB302_H
