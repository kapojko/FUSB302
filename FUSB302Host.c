#include "FUSB302Host.h"

bool FUSB302_SetupHostMonitoring(FUSB302_Platform_t *platform, FUSB302_Data_t *data,
                                 FUSB302_HostCurrentMode_t hostCurrentMode,
                                 FUSB302_HostMonitoring_t *monitoring) {
    // Read control data
    if (!FUSB302_ReadControlData(platform, data, FUSB302_REG_ALL)) {
        return false;
    }

    // Enable host pullups (note: both pullups seem to be internally connected ~300R)
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

            bool ok = true;

            // Set switches for CC1 measure only (no pull-up for CC2 since pull-ups are connected)
            FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_PU_EN1, 1);
            FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_PU_EN2, 0);
            FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_MEAS_CC1, 1);
            FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_MEAS_CC2, 0);
            ok &= FUSB302_WriteControlData(platform, data, FUSB302_REG_SWITCHES0);

            platform->delayUs(1000);

            // Read BC_LVL for CC1
            ok &= FUSB302_ReadStatusData(platform, data, FUSB302_REG_STATUS0);
            uint8_t bc_lvl_cc1 = FUSB302_GetDataValue(data, FUSB302_REG_STATUS0, FUSB302_BC_LVL_BITS,
                                                      FUSB302_BC_LVL_OFFSET);

            // Set switches for CC2 measure only (no pull-up for CC1 since pull-ups are connected)
            FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_PU_EN1, 0);
            FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_PU_EN2, 1);
            FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_MEAS_CC1, 0);
            FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_MEAS_CC2, 1);
            ok &= FUSB302_WriteControlData(platform, data, FUSB302_REG_SWITCHES0);

            platform->delayUs(1000);

            // Read BC_LVL for CC2
            ok &= FUSB302_ReadStatusData(platform, data, FUSB302_REG_STATUS0);
            uint8_t bc_lvl_cc2 = FUSB302_GetDataValue(data, FUSB302_REG_STATUS0, FUSB302_BC_LVL_BITS,
                                                      FUSB302_BC_LVL_OFFSET);

            // Restore switches configuration
            FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_PU_EN1, 1);
            FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_PU_EN2, 1);
            FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_MEAS_CC1, 1);
            FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_MEAS_CC2, 0);
            ok &= FUSB302_WriteControlData(platform, data, FUSB302_REG_SWITCHES0);

            platform->delayUs(1000);

            // Read interrupt status register to clear interrupts
            ok &= FUSB302_ReadStatusData(platform, data, FUSB302_REG_INTERRUPT);

            // Check error
            if (!ok) {
                return false;
            }

            // platform->debugPrint("FUSB302 device attached: COMP=%d bc_lvl_cc1=%d bc_lvl_cc2=%d\r\n", comp, bc_lvl_cc1, bc_lvl_cc2);

            // Determine CC level
            if (bc_lvl_cc1 == FUSB302_BC_LVL_1230MV_MORE && bc_lvl_cc2 != FUSB302_BC_LVL_1230MV_MORE) {
                monitoring->ccOrientation = FUSB302_CC_ORIENTATION_CC2;
            } else if (bc_lvl_cc2 == FUSB302_BC_LVL_1230MV_MORE && bc_lvl_cc1 != FUSB302_BC_LVL_1230MV_MORE) {
                monitoring->ccOrientation = FUSB302_CC_ORIENTATION_CC1;
            } else {
                monitoring->ccOrientation = FUSB302_CC_ORIENTATION_UNKNOWN;
            }
        }
    }

    return true;
}
