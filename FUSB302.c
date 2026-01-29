#include "FUSB302.h"

#define I2C_TIMEOUT 1

bool FUSB302_ReadControlData(FUSB302_Platform_t *platform, FUSB302_Data_t *data, int reg) {
    // Read register data
    int ret;
    if (reg != FUSB302_REG_ALL) {
        ret = platform->i2cReadReg(FUSB302_I2C_ADDR, reg,
                                   &data->controlRegData[reg - FUSB302_REG_CONTROL_START], 1,
                                   I2C_TIMEOUT);
    } else {
        ret = platform->i2cReadReg(FUSB302_I2C_ADDR, FUSB302_REG_CONTROL_START,
                                   data->controlRegData, FUSB302_REG_CONTROL_NUM, I2C_TIMEOUT);
    }

    if (ret < 0) {
        return false;
    }

    return true;
}

bool FUSB302_WriteControlData(FUSB302_Platform_t *platform, FUSB302_Data_t *data, int reg) {
    // Write register data
    int ret;
    if (reg != FUSB302_REG_ALL) {
        ret = platform->i2cWriteReg(FUSB302_I2C_ADDR, reg,
                                    &data->controlRegData[reg - FUSB302_REG_CONTROL_START], 1,
                                    I2C_TIMEOUT);
    } else {
        ret = platform->i2cWriteReg(FUSB302_I2C_ADDR, FUSB302_REG_CONTROL_START,
                                    data->controlRegData, FUSB302_REG_CONTROL_NUM, I2C_TIMEOUT);
    }

    if (ret < 0) {
        return false;
    }

    return true;
}

void FUSB302_DebugPrintControlData(FUSB302_Platform_t *platform, FUSB302_Data_t *data, int reg) {
    platform->debugPrint("FUSB302 Control:");

    if (reg != FUSB302_REG_ALL) {
        platform->debugPrint(" %02XH=0x%02X", reg,
                             data->controlRegData[reg - FUSB302_REG_CONTROL_START]);
    } else {
        for (int i = FUSB302_REG_CONTROL_START;
             i < FUSB302_REG_CONTROL_START + FUSB302_REG_CONTROL_NUM; i++) {
            platform->debugPrint(" %02XH=0x%02X", i,
                                 data->controlRegData[i - FUSB302_REG_CONTROL_START]);
        }
    }

    platform->debugPrint("\r\n");
}

bool FUSB302_ReadStatusData(FUSB302_Platform_t *platform, FUSB302_Data_t *data, int reg) {
    int ret;
    if (reg != FUSB302_REG_ALL) {
        ret = platform->i2cReadReg(FUSB302_I2C_ADDR, reg,
                                   &data->statusRegData[reg - FUSB302_REG_STATUS_START], 1,
                                   I2C_TIMEOUT);
    } else {
        ret = platform->i2cReadReg(FUSB302_I2C_ADDR, FUSB302_REG_STATUS_START, data->statusRegData,
                                   FUSB302_REG_STATUS_NUM, I2C_TIMEOUT);
    }

    if (ret < 0) {
        return false;
    }

    return true;
}

void FUSB302_DebugPrintStatusData(FUSB302_Platform_t *platform, FUSB302_Data_t *data, int reg) {
    platform->debugPrint("FUSB302 Status:");

    if (reg != FUSB302_REG_ALL) {
        platform->debugPrint(" %02XH=0x%02X", reg,
                             data->statusRegData[reg - FUSB302_REG_STATUS_START]);
    } else {
        for (int i = FUSB302_REG_STATUS_START;
             i < FUSB302_REG_STATUS_START + FUSB302_REG_STATUS_NUM; i++) {
            platform->debugPrint(" %02XH=0x%02X", i,
                                 data->statusRegData[i - FUSB302_REG_STATUS_START]);
        }
    }

    platform->debugPrint("\r\n");
}

bool FUSB302_ReadFIFO(FUSB302_Platform_t *platform, uint8_t *data, uint8_t length) {
    return platform->i2cReadReg(FUSB302_I2C_ADDR, FUSB302_REG_FIFOS, data, length, I2C_TIMEOUT) >=
           0;
}

bool FUSB302_WriteFIFO(FUSB302_Platform_t *platform, uint8_t *data, uint8_t length) {
    return platform->i2cWriteReg(FUSB302_I2C_ADDR, FUSB302_REG_FIFOS, data, length, I2C_TIMEOUT) >=
           0;
}

uint8_t *FUSB302_GetRegPtr(FUSB302_Data_t *data, int reg) {
    if (reg >= FUSB302_REG_CONTROL_START &&
        reg < FUSB302_REG_CONTROL_START + FUSB302_REG_CONTROL_NUM) {
        return &data->controlRegData[reg - FUSB302_REG_CONTROL_START];
    } else if (reg >= FUSB302_REG_STATUS_START &&
               reg < FUSB302_REG_STATUS_START + FUSB302_REG_STATUS_NUM) {
        return &data->statusRegData[reg - FUSB302_REG_STATUS_START];
    } else {
        return 0;
    }
}

int FUSB302_GetDataBit(FUSB302_Data_t *data, int reg, int bitMask) {
    // Get register data value
    uint8_t *regData = FUSB302_GetRegPtr(data, reg);
    if (!regData) {
        return -1;
    }

    // Single bit value
    return (*regData & bitMask) ? 1 : 0;
}

void FUSB302_SetDataBit(FUSB302_Data_t *data, int reg, int bitMask, int value) {
    // Get register data value
    uint8_t *regData = FUSB302_GetRegPtr(data, reg);
    if (!regData) {
        return;
    }

    // Single bit value
    *regData = (*regData & ~bitMask) | (value ? bitMask : 0);
}

int FUSB302_GetDataValue(FUSB302_Data_t *data, int reg, int bitMask, int offset) {
    // Get register data value
    uint8_t *regData = FUSB302_GetRegPtr(data, reg);
    if (!regData) {
        return -1;
    }

    // Multiple bit value
    return (*regData & bitMask) >> offset;
}

void FUSB302_SetDataValue(FUSB302_Data_t *data, int reg, int bitMask, int offset, int value) {
    // Get register data value
    uint8_t *regData = FUSB302_GetRegPtr(data, reg);
    if (!regData) {
        return;
    }

    // Multiple bit value
    *regData = (*regData & ~bitMask) | ((value << offset) & bitMask);
}

bool FUSB302_SetupToggleMode(FUSB302_Platform_t *platform, FUSB302_Data_t *data,
                             FUSB302_ToggleMode_t mode, FUSB302_HostCurrentMode_t hostCurrentMode) {
    // Read control data
    if (!FUSB302_ReadControlData(platform, data, FUSB302_REG_ALL)) {
        return false;
    }

    if (mode != FUSB302_TOGGLE_MODE_MANUAL) {
        // Read status data to clear interrupt
        if (!FUSB302_ReadStatusData(platform, data, FUSB302_REG_ALL)) {
            return false;
        }

        // Configure toggle mode
        int modeValue;
        switch (mode) {
        case FUSB302_TOGGLE_MODE_DRP:
            modeValue = FUSB302_MODE_TOGGLE_DRP;
            break;
        case FUSB302_TOGGLE_MODE_SNK:
            modeValue = FUSB302_MODE_TOGGLE_SNK;
            break;
        case FUSB302_TOGGLE_MODE_SRC:
            modeValue = FUSB302_MODE_TOGGLE_SRC;
            break;
        default:
            return false;
        }

        FUSB302_SetDataValue(data, FUSB302_REG_CONTROL2, FUSB302_MODE_BITS, FUSB302_MODE_OFFSET,
                             modeValue);

        // Set toggle mode (TOGGLE=1)
        FUSB302_SetDataBit(data, FUSB302_REG_CONTROL2, FUSB302_TOGGLE, 1);

        // Setup power (PWR=07H)
        FUSB302_SetDataBit(data, FUSB302_REG_POWER, FUSB302_PWR_MEAS_BLOCK, 1);
        FUSB302_SetDataBit(data, FUSB302_REG_POWER, FUSB302_PWR_RECV_CUR, 1);
        FUSB302_SetDataBit(data, FUSB302_REG_POWER, FUSB302_PWR_BANDGAP_WAKE, 1);

        // Setup host current (default HUST_CUR=01b)
        int hostCurValue;
        switch (hostCurrentMode) {
        case FUSB302_HOST_CURRENT_MODE_500MA:
            hostCurValue = FUSB302_HOST_CUR_DEF_USB;
            break;
        case FUSB302_HOST_CURRENT_MODE_1_5A:
            hostCurValue = FUSB302_HOST_CUR_1_5A;
            break;
        case FUSB302_HOST_CURRENT_MODE_3A:
            hostCurValue = FUSB302_HOST_CUR_3A;
            break;
        default:
            hostCurValue = FUSB302_HOST_CUR_DEF_USB;
            break;
        }

        FUSB302_SetDataValue(data, FUSB302_REG_CONTROL0, FUSB302_HOST_CUR_BITS,
                             FUSB302_HOST_CUR_OFFSET, hostCurValue);

        // Setup VBUS measurement (MEAS_VBUS=0)
        FUSB302_SetDataBit(data, FUSB302_REG_MEASURE, FUSB302_MEAS_VBUS, 0);

        // Setup VCONN (VCONN_CC1=0, VCONN_CC2=0)
        FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_VCONN_CC1, 0);
        FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_VCONN_CC2, 0);

        // Setup interrupt mask (only I_TOGDONE and I_BC_LVL Interrupt)
        // Mask Register = 0xFE
        FUSB302_SetDataValue(data, FUSB302_REG_MASK, 0xFF, 0, ~FUSB302_M_BC_LVL);
        // Maska Register = 0xBF
        FUSB302_SetDataValue(data, FUSB302_REG_MASKA, 0xFF, 0, ~FUSB302_M_TOGDONE);
        // Maskb Register = 0x01
        FUSB302_SetDataBit(data, FUSB302_REG_MASKB, FUSB302_M_GCRCSENT, 1);

        // TODO: check
        // Set Rd only for TOGGLE
        FUSB302_SetDataBit(data, FUSB302_REG_CONTROL2, FUSB302_TOG_RD_ONLY, 1);

        // TODO: check
        // Enable host interrupts (INT_MASK=0)
        FUSB302_SetDataBit(data, FUSB302_REG_CONTROL0, FUSB302_INT_MASK, 0);
    } else {
        // Reset toggle mode (TOGGLE=0)
        FUSB302_SetDataBit(data, FUSB302_REG_CONTROL2, FUSB302_TOGGLE, 0);
    }

    // Write control data
    if (!FUSB302_WriteControlData(platform, data, FUSB302_REG_ALL)) {
        return false;
    }

    return true;
}

bool FUSB302_GetToggleResult(FUSB302_Platform_t *platform, FUSB302_Data_t *data,
                             FUSB302_ToggleResult_t *result) {
    // Read status registers
    if (!FUSB302_ReadStatusData(platform, data, FUSB302_REG_ALL)) {
        return false;
    }

    // Check interrupt status
    int i_togdone = FUSB302_GetDataBit(data, FUSB302_REG_INTERRUPTA, FUSB302_I_TOGDONE);
    int i_bclvl = FUSB302_GetDataBit(data, FUSB302_REG_INTERRUPT, FUSB302_I_BC_LVL);

    if (i_togdone) {
        platform->debugPrint("FUSB302: Toggle done\r\n");
    }
    if (i_bclvl) {
        platform->debugPrint("FUSB302: BC level\r\n");
    }

    if (!i_togdone) {
        *result = FUSB302_TOGGLE_RESULT_NONE;
        return true;
    }

    // Check toggle result
    int togss =
        FUSB302_GetDataValue(data, FUSB302_REG_STATUS1A, FUSB302_TOGSS_BITS, FUSB302_TOGSS_OFFSET);
    switch (togss) {
    case FUSB302_TOGSS_RUNNING:
        *result = FUSB302_TOGGLE_RESULT_NONE;
        break;
    case FUSB302_TOGSS_STOP_SRC1:
        *result = FUSB302_TOGGLE_RESULT_SRC_CC1;
        break;
    case FUSB302_TOGSS_STOP_SRC2:
        *result = FUSB302_TOGGLE_RESULT_SRC_CC2;
        break;
    case FUSB302_TOGSS_STOP_SNK1:
        *result = FUSB302_TOGGLE_RESULT_SNK_CC1;
        break;
    case FUSB302_TOGSS_STOP_SNK2:
        *result = FUSB302_TOGGLE_RESULT_SNK_CC2;
        break;
    case FUSB302_TOGSS_AUDIO:
        *result = FUSB302_TOGGLE_RESULT_AUDIO;
        break;
    default:
        return false;
    }

    return true;
}

bool FUSB302_SetupHostMonitoring(FUSB302_Platform_t *platform, FUSB302_Data_t *data,
                                 FUSB302_HostCurrentMode_t hostCurrentMode,
                                 FUSB302_HostMonitoring_t *monitoring) {
    // Read control data
    if (!FUSB302_ReadControlData(platform, data, FUSB302_REG_ALL)) {
        return false;
    }

    // Enable host pullups
    FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_PU_EN1, 1);
    FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_PU_EN2, 1);

    // Setup current CC to measure
    FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_MEAS_CC1, 1);
    FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_MEAS_CC2, 0);

    // Disable host powerdowns
    FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_PDWN1, 0);
    FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_PDWN2, 0);

    // Setup comparator to measure CC
    FUSB302_SetDataBit(data, FUSB302_REG_MEASURE, FUSB302_MEAS_VBUS, 0);

    // Setup Measure Block DAC data input
    if (hostCurrentMode != FUSB302_HOST_CURRENT_MODE_3A) {
        // 6'b10_0101 (1.6 V)
        uint8_t mdacValue = 0b100101;
        FUSB302_SetDataValue(data, FUSB302_REG_MEASURE, FUSB302_MDAC_BITS, FUSB302_MDAC_OFFSET,
                             mdacValue);
    } else {
        // 6'b11_1101 (2.6 V)
        uint8_t mdacValue = 0b111101;
        FUSB302_SetDataValue(data, FUSB302_REG_MEASURE, FUSB302_MDAC_BITS, FUSB302_MDAC_OFFSET,
                             mdacValue);
    }

    // Enable interrupts to host
    FUSB302_SetDataBit(data, FUSB302_REG_CONTROL0, FUSB302_INT_MASK, 0);

    // Setup host current
    int hostCurValue;
    switch (hostCurrentMode) {
    case FUSB302_HOST_CURRENT_MODE_500MA:
        hostCurValue = FUSB302_HOST_CUR_DEF_USB;
        break;
    case FUSB302_HOST_CURRENT_MODE_1_5A:
        hostCurValue = FUSB302_HOST_CUR_1_5A;
        break;
    case FUSB302_HOST_CURRENT_MODE_3A:
        hostCurValue = FUSB302_HOST_CUR_3A;
        break;
    default:
        hostCurValue = FUSB302_HOST_CUR_DEF_USB;
        break;
    }

    FUSB302_SetDataValue(data, FUSB302_REG_CONTROL0, FUSB302_HOST_CUR_BITS, FUSB302_HOST_CUR_OFFSET,
                         hostCurValue);

    // Mask all interupts except M_COMP_CHNG and M_BC_LVL
    FUSB302_SetDataValue(data, FUSB302_REG_MASK, 0xFF, 0, ~(FUSB302_M_COMP_CHNG | FUSB302_M_BC_LVL));
    FUSB302_SetDataValue(data, FUSB302_REG_MASKA, 0xFF, 0, 0xFF);
    FUSB302_SetDataBit(data, FUSB302_REG_MASKB, FUSB302_M_GCRCSENT, 1);

    // Setup power
    FUSB302_SetDataBit(data, FUSB302_REG_POWER, FUSB302_PWR_MEAS_BLOCK, 1);
    FUSB302_SetDataBit(data, FUSB302_REG_POWER, FUSB302_PWR_RECV_CUR, 1);
    FUSB302_SetDataBit(data, FUSB302_REG_POWER, FUSB302_PWR_BANDGAP_WAKE, 1);

    // Write control registers
    if (!FUSB302_WriteControlData(platform, data, FUSB302_REG_ALL)) {
        return false;
    }

    // Set initial monitoring state
    monitoring->state = FUSB302_HOST_STATE_INIT;

    return true;
}

bool FUSB302_UpdateHostMonitoring(FUSB302_Platform_t *platform, FUSB302_Data_t *data,
                                  FUSB302_HostMonitoring_t *monitoring) {
    // Read interrupt status registr
    if (!FUSB302_ReadStatusData(platform, data, FUSB302_REG_INTERRUPT)) {
        return false;
    }

    // Check COMP and BC_LVL interrupt
    uint8_t i_comp_chng = FUSB302_GetDataBit(data, FUSB302_REG_INTERRUPT, FUSB302_I_COMP_CHNG);
    uint8_t i_bc_lvl = FUSB302_GetDataBit(data, FUSB302_REG_INTERRUPT, FUSB302_I_BC_LVL);
    if (i_comp_chng || i_bc_lvl || monitoring->state == FUSB302_HOST_STATE_INIT) {
        // Read STATUS0 register
        if (!FUSB302_ReadStatusData(platform, data, FUSB302_REG_STATUS0)) {
            return false;
        }

        // Check COMP value
        uint8_t comp = FUSB302_GetDataBit(data, FUSB302_REG_STATUS0, FUSB302_COMP);
        if (comp) {
            // 1: Measured CC* input is higher than reference level driven from the MDAC.

            monitoring->state = FUSB302_HOST_STATE_DEVICE_DETACHED;
        } else {
            // 0: Measured CC* input is lower than reference level driven from the MDAC.

            monitoring->state = FUSB302_HOST_STATE_DEVICE_ATTACHED;

            // Check BC_LVL
            uint8_t bc_lvl = FUSB302_GetDataValue(data, FUSB302_REG_STATUS0, FUSB302_BC_LVL_BITS,
                                                  FUSB302_BC_LVL_OFFSET);
            if (bc_lvl == FUSB302_BC_LVL_0_200MV) {
                // Another CC line triggered
                monitoring->ccOrientation = FUSB302_CC_ORIENTATION_CC2;
            } else {
                monitoring->ccOrientation = FUSB302_CC_ORIENTATION_CC1;
            }
        }
    }

    return true;
}

bool FUSB302_Reset(FUSB302_Platform_t *platform, FUSB302_Data_t *data) {
    // Setup data: Reset the FUSB302 including the I2C registers to their default values.
    FUSB302_SetDataBit(data, FUSB302_REG_RESET, FUSB302_SW_RESET, 1);

    // Write updated register
    return FUSB302_WriteControlData(platform, data, FUSB302_REG_RESET);
}
