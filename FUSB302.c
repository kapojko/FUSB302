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

bool FUSB302_Reset(FUSB302_Platform_t *platform, FUSB302_Data_t *data) {
    // Setup data: Reset the FUSB302 including the I2C registers to their default values.
    FUSB302_SetDataBit(data, FUSB302_REG_RESET, FUSB302_SW_RESET, 1);

    // Write updated register
    return FUSB302_WriteControlData(platform, data, FUSB302_REG_RESET);
}
