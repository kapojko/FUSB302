#include "FUSB302Toggle.h"

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
