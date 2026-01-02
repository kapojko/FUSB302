#include "FUSB302.h"

#define I2C_TIMEOUT 1

static uint8_t *GetRegPtr(FUSB302_Data_t *data, int reg) {
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

// Write register data
int ret;
if (reg != FUSB302_REG_ALL) {
    ret = platform->i2cWriteReg(FUSB302_I2C_ADDR, reg,
                                &data->controlRegData[reg - FUSB302_REG_CONTROL_START], 1,
                                I2C_TIMEOUT);
} else {
    ret = platform->i2cWriteReg(FUSB302_I2C_ADDR, FUSB302_REG_CONTROL_START, data->controlRegData,
                                FUSB302_REG_CONTROL_NUM, I2C_TIMEOUT);
}

if (ret < 0) {
    return false;
}

return true;
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

bool FUSB302_ReadFIFO(FUSB302_Platform_t *platform, uint8_t *data, uint8_t length) {
    return platform->i2cReadReg(FUSB302_I2C_ADDR, FUSB302_REG_FIFOS, data, length, I2C_TIMEOUT) >=
           0;
}

bool FUSB302_WriteFIFO(FUSB302_Platform_t *platform, uint8_t *data, uint8_t length) {
    return platform->i2cWriteReg(FUSB302_I2C_ADDR, FUSB302_REG_FIFOS, data, length, I2C_TIMEOUT) >=
           0;
}

int FUSB302_GetDataValue(FUSB302_Data_t *data, int reg, int bitMask, int offset) {
    // Get register data value
    uint8_t *regData = GetRegPtr(data, reg);
    if (!regData) {
        return -1;
    }

    // Extract value
    if (offset == FUSB302_OFFSET_NONE) {
        // Single bit value
        return (*regData & bitMask) ? 1 : 0;
    } else {
        // Multiple bit value
        return (*regData & bitMask) >> offset;
    }
}

void FUSB302_SetDataValue(FUSB302_Data_t *data, int reg, int bitMask, int offset, int value) {
    // Get register data value
    uint8_t *regData = GetRegPtr(data, reg);
    if (!regData) {
        return;
    }

    // Set value
    if (offset == FUSB302_OFFSET_NONE) {
        // Single bit value
        *regData = (*regData & ~bitMask) | (value ? bitMask : 0);
    } else {
        // Multiple bit value
        *regData = (*regData & ~bitMask) | ((value << offset) & bitMask);
    }
}

bool FUSB302_SetToggleMode(FUSB302_Platform_t *platform, FUSB302_Data_t *data, FUSB302_ToggleMode_t mode) {
    // Read control data
    if (!FUSB302_ReadControlData(platform, data, FUSB302_REG_ALL)) {
        return false;
    }

    if (mode != FUSB302_TOGGLE_MODE_MANUAL) {
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
        FUSB302_SetDataValue(data, FUSB302_REG_CONTROL2, FUSB302_TOGGLE, FUSB302_OFFSET_NONE, 1);

        // Setup power (PWR=07H)
        FUSB302_SetDataValue(data, FUSB302_REG_POWER, FUSB302_PWR_MEAS_BLOCK, FUSB302_OFFSET_NONE,
                             1);
        FUSB302_SetDataValue(data, FUSB302_REG_POWER, FUSB302_PWR_RECV_CUR, FUSB302_OFFSET_NONE,
                             1);
        FUSB302_SetDataValue(data, FUSB302_REG_POWER, FUSB302_PWR_BANDGAP_WAKE,
                             FUSB302_OFFSET_NONE, 1);

        // Setup host current (HUST_CUR=01b)
        FUSB302_SetDataValue(data, FUSB302_REG_CONTROL0, FUSB302_HOST_CUR_BITS,
                             FUSB302_HOST_CUR_OFFSET, FUSB302_HOST_CUR_DEF_USB);

        // Setup VBUS measurement (MEAS_VBUS=0)
        FUSB302_SetDataValue(data, FUSB302_REG_MEASURE, FUSB302_MEAS_VBUS, FUSB302_OFFSET_NONE, 0);

        // Setup VCONN (VCONN_CC1=0, VCONN_CC2=0)
        FUSB302_SetDataValue(data, FUSB302_REG_SWITCHES0, FUSB302_VCONN_CC1, FUSB302_OFFSET_NONE,
                             0);
        FUSB302_SetDataValue(data, FUSB302_REG_SWITCHES0, FUSB302_VCONN_CC2, FUSB302_OFFSET_NONE,
                             0);

        // Setup interrupt mask (only I_TOGDONE and I_BC_LVL Interrupt)
        // Mask Register = 0xFE
        FUSB302_SetDataValue(data, FUSB302_REG_MASK, 0xFF, 0, ~FUSB302_M_BC_LVL);
        // Maska Register = 0xBF
        FUSB302_SetDataValue(data, FUSB302_REG_MASKA, 0xFF, 0, ~FUSB302_M_TOGDONE);
        // Maskb Register = 0x01
        FUSB302_SetDataValue(data, FUSB302_REG_MASKB, FUSB302_M_GCRCSENT, FUSB302_OFFSET_NONE, 1);
    } else {
        // Reset toggle mode (TOGGLE=0)
        FUSB302_SetDataValue(data, FUSB302_REG_CONTROL2, FUSB302_TOGGLE, FUSB302_OFFSET_NONE, 0);
    }

    // Write control data
    if (!FUSB302_WriteControlData(platform, data, FUSB302_REG_ALL)) {
        return false;
    }

    return true;
}

bool FUSB302_GetToggleResult(FUSB302_Platform_t *platform, FUSB302_Data_t *data, FUSB302_ToggleResult_t *result) {
    // Read status registers
    if (!FUSB302_ReadStatusData(platform, data, FUSB302_REG_ALL)) {
        return false;
    }

    // Check interrupt status
    int i_togdone = FUSB302_GetDataValue(data, FUSB302_REG_INTERRUPTA, FUSB302_I_TOGDONE, FUSB302_OFFSET_NONE);
    // int i_bclvl = FUSB302_GetDataValue(data, FUSB302_REG_INTERRUPT, FUSB302_I_BC_LVL, FUSB302_OFFSET_NONE);

    if (!i_togdone) {
        *result = FUSB302_TOGGLE_RESULT_NONE;
        return true;
    }

    // Check toggle result
    int togss = FUSB302_GetDataValue(data, FUSB302_REG_STATUS1A, FUSB302_TOGSS_BITS, FUSB302_TOGSS_OFFSET);
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

bool FUSB302_Reset(FUSB302_Platform_t *platform, FUSB302_Data_t *data) {
    // TODO: implement
    return false;
}
