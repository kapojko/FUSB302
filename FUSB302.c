#include "FUSB302.h"

#define I2C_TIMEOUT 1

bool FUSB302_ReadControlData(FUSB302_Platform_t *platform, FUSB302_ControlData_t *controlData,
                             int reg) {
    // Read register data
    int ret;
    if (reg != FUSB302_REG_CONTROL_ALL) {
        ret = platform->i2cReadReg(FUSB302_I2C_ADDR, reg,
                                   &controlData->regData[reg - FUSB302_REG_CONTROL_START], 1,
                                   I2C_TIMEOUT);
    } else {
        ret = platform->i2cReadReg(FUSB302_I2C_ADDR, FUSB302_REG_CONTROL_START,
                                   controlData->regData, FUSB302_REG_CONTROL_NUM, I2C_TIMEOUT);
    }

    if (ret < 0) {
        return false;
    }

    return true;
}

bool FUSB302_WriteControlData(FUSB302_Platform_t *platform, FUSB302_ControlData_t *controlData,
                              int reg) {
    // Write register data
    int ret;
    if (reg != FUSB302_REG_CONTROL_ALL) {
        ret = platform->i2cWriteReg(FUSB302_I2C_ADDR, reg,
                                    &controlData->regData[reg - FUSB302_REG_CONTROL_START], 1,
                                    I2C_TIMEOUT);
    } else {
        ret = platform->i2cWriteReg(FUSB302_I2C_ADDR, FUSB302_REG_CONTROL_START,
                                    controlData->regData, FUSB302_REG_CONTROL_NUM, I2C_TIMEOUT);
    }

    if (ret < 0) {
        return false;
    }

    return true;
}

bool FUSB302_ReadStatusData(FUSB302_Platform_t *platform, FUSB302_StatusData_t *statusData,
                            int reg) {
    int ret;
    if (reg != FUSB302_REG_STATUS_ALL) {
        ret = platform->i2cReadReg(FUSB302_I2C_ADDR, reg,
                                   &statusData->regData[reg - FUSB302_REG_STATUS_START], 1,
                                   I2C_TIMEOUT);
    } else {
        ret = platform->i2cReadReg(FUSB302_I2C_ADDR, FUSB302_REG_STATUS_START, statusData->regData,
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

bool FUSB302_SetToggleMode(FUSB302_Platform_t *platform, FUSB302_ToggleMode_t mode) {
    // TODO: implement
    return false;
}

bool FUSB302_GetToggleResult(FUSB302_Platform_t *platform, FUSB302_ToggleResult_t *result) {
    // TODO: implement
    return false;
}

bool FUSB302_Reset(FUSB302_Platform_t *platform) {
    // TODO: implement
    return false;
}
