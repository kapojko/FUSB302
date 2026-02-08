#include "FUSB302Host.h"
#include "FUSB302/FUSB302.h"

// #define EXPERIMENTAL_EMARKER_READ

static bool DiscoverCCOrientation(FUSB302_Platform_t *platform, FUSB302_Data_t *data,
                                  FUSB302_HostMonitoring_t *monitoring) {
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
    uint8_t bc_lvl_cc1 =
        FUSB302_GetDataValue(data, FUSB302_REG_STATUS0, FUSB302_BC_LVL_BITS, FUSB302_BC_LVL_OFFSET);

    // Set switches for CC2 measure only (no pull-up for CC1 since pull-ups are connected)
    FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_PU_EN1, 0);
    FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_PU_EN2, 1);
    FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_MEAS_CC1, 0);
    FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_MEAS_CC2, 1);
    ok &= FUSB302_WriteControlData(platform, data, FUSB302_REG_SWITCHES0);

    platform->delayUs(1000);

    // Read BC_LVL for CC2
    ok &= FUSB302_ReadStatusData(platform, data, FUSB302_REG_STATUS0);
    uint8_t bc_lvl_cc2 =
        FUSB302_GetDataValue(data, FUSB302_REG_STATUS0, FUSB302_BC_LVL_BITS, FUSB302_BC_LVL_OFFSET);

    // Check error
    if (!ok) {
        return false;
    }

    // Determine expected CC level (depending on host current mode)
    uint8_t expectedBcLvl_Rd;
    uint8_t expectedBcLvl_Ra, expectedBcLvl_Ra_alt;
    uint8_t expectedBcLvl_Nc = FUSB302_BC_LVL_1230MV_MORE;
    switch (monitoring->hostCurrentMode) {
    case FUSB302_HOST_CURRENT_MODE_500MA:
        expectedBcLvl_Rd = FUSB302_BC_LVL_200_660MV; // 0.41 V
        expectedBcLvl_Ra = FUSB302_BC_LVL_0_200MV;   // 0.08 V
        expectedBcLvl_Ra_alt = expectedBcLvl_Ra;     // no alternative
        break;
    case FUSB302_HOST_CURRENT_MODE_1_5A:
        expectedBcLvl_Rd = FUSB302_BC_LVL_660_1230MV;    // 0.92 V
        expectedBcLvl_Ra = FUSB302_BC_LVL_0_200MV;       // 0.18 V
        expectedBcLvl_Ra_alt = FUSB302_BC_LVL_200_660MV; // also possible
        break;
    case FUSB302_HOST_CURRENT_MODE_3A:
        expectedBcLvl_Rd = FUSB302_BC_LVL_1230MV_MORE; // 1.68 V
        expectedBcLvl_Ra = FUSB302_BC_LVL_200_660MV;   // 0.33 V
        expectedBcLvl_Ra_alt = expectedBcLvl_Ra;       // no alternative
        break;
    }

    // Check CC orientation and cable type
    if (bc_lvl_cc1 == expectedBcLvl_Rd && bc_lvl_cc2 == expectedBcLvl_Nc) {
        // Device on CC1, passive cable
        monitoring->ccOrientation = FUSB302_CC_ORIENTATION_CC1;
        monitoring->cableType = FUSB302_CABLE_TYPE_PASSIVE;
    } else if (bc_lvl_cc2 == expectedBcLvl_Rd && bc_lvl_cc1 == expectedBcLvl_Nc) {
        // Device on CC2, passive cable
        monitoring->ccOrientation = FUSB302_CC_ORIENTATION_CC2;
        monitoring->cableType = FUSB302_CABLE_TYPE_PASSIVE;
    } else if (bc_lvl_cc1 == expectedBcLvl_Rd &&
               (bc_lvl_cc2 == expectedBcLvl_Ra || bc_lvl_cc2 == expectedBcLvl_Ra_alt)) {
        // Device on CC1, active cable
        monitoring->ccOrientation = FUSB302_CC_ORIENTATION_CC1;
        monitoring->cableType = FUSB302_CABLE_TYPE_EMARKER;
    } else if (bc_lvl_cc2 == expectedBcLvl_Rd &&
               (bc_lvl_cc1 == expectedBcLvl_Ra || bc_lvl_cc1 == expectedBcLvl_Ra_alt)) {
        // Device on CC2, active cable
        monitoring->ccOrientation = FUSB302_CC_ORIENTATION_CC2;
        monitoring->cableType = FUSB302_CABLE_TYPE_EMARKER;
    } else if (bc_lvl_cc1 == expectedBcLvl_Nc &&
               (bc_lvl_cc2 == expectedBcLvl_Ra || bc_lvl_cc2 == expectedBcLvl_Ra_alt)) {
        // Device should be on CC1, but only active cable connected, no device
        monitoring->ccOrientation = FUSB302_CC_ORIENTATION_CC1;
        monitoring->cableType = FUSB302_CABLE_TYPE_EMARKER_ONLY;
    } else if (bc_lvl_cc2 == expectedBcLvl_Nc &&
               (bc_lvl_cc1 == expectedBcLvl_Ra || bc_lvl_cc1 == expectedBcLvl_Ra_alt)) {
        // Device should be on CC2, but only active cable connected, no device
        monitoring->ccOrientation = FUSB302_CC_ORIENTATION_CC2;
        monitoring->cableType = FUSB302_CABLE_TYPE_EMARKER_ONLY;
    } else {
        // Unknown
        monitoring->ccOrientation = FUSB302_CC_ORIENTATION_UNKNOWN;
        monitoring->cableType = FUSB302_CABLE_TYPE_UNKNOWN;
    }

#ifdef FUSB302_DEBUG
    platform->debugPrint(
        "FUSB302: ccOrientation=%d cableType=%d (bc_lbl_cc1=%d, bc_lbl_cc2=%d)\r\n",
        monitoring->ccOrientation, monitoring->cableType, bc_lvl_cc1, bc_lvl_cc2);
#endif

    return true;
}

#ifdef EXPERIMENTAL_EMARKER_READ
static bool SendDiscoveryIdentity(FUSB302_Platform_t *platform, FUSB302_Data_t *data,
                                  FUSB302_HostMonitoring_t *monitoring, uint8_t *txBuffer,
                                  int txBufferSize) {
    // Build Discover Identity message for cable (SOP')

    uint8_t txLen = 0;

    // SOP' token sequence for cable communication
    txBuffer[txLen++] = FUSB302_TOKEN_SOP1; // SOP1
    txBuffer[txLen++] = FUSB302_TOKEN_SOP1; // SOP1
    txBuffer[txLen++] = FUSB302_TOKEN_SOP3; // SOP3
    txBuffer[txLen++] = FUSB302_TOKEN_SOP3; // SOP3

    // PACKSYM token with 2 bytes (header only, no data objects for Discover Identity)
    // PACKSYM encoding: 0x80 | number_of_bytes (N must be 2-30)
    txBuffer[txLen++] = FUSB302_TOKEN_PACKSYM | 0x02;

    // PD Header: Message Type 1 (Discover Identity), Port Data Role=Source(1), Port Power
    txBuffer[txLen++] = 0x01;  // Message Type 1, Cable Plug 0
    txBuffer[txLen++] = 0x05;  // Message ID 0, Source, PD 2.0

    // JAM_CRC token - hardware will calculate and insert CRC
    txBuffer[txLen++] = FUSB302_TOKEN_JAM_CRC;

    // EOP token
    txBuffer[txLen++] = FUSB302_TOKEN_EOP;

    // TXOFF token - turn off transmitter after EOP
    txBuffer[txLen++] = FUSB302_TOKEN_TXOFF;

    // TXON token - start transmitter (MUST be last!)
    // Per datasheet: "It is preferred that the TxFIFO is first written with data
    // and then TXON or TX_START is executed."
    txBuffer[txLen++] = FUSB302_TOKEN_TXON;

    // Write TX FIFO with all tokens (TXON at end)
    if (!FUSB302_WriteFIFO(platform, txBuffer, txLen)) {
        return false;
    }

    return true;
}

static bool ReadDiscoveryIdentity(FUSB302_Platform_t *platform, FUSB302_Data_t *data,
                                  FUSB302_HostMonitoring_t *monitoring, uint8_t *rxBuffer,
                                  int rxBufferSize) {
    // Read response from RX FIFO
    uint8_t rxLen = 0;

    // Read first byte to determine SOP type
    if (!FUSB302_ReadFIFO(platform, &rxBuffer[rxLen], 1)) {
        return false;
    }
    rxLen++;

    // Check SOP type (upper 3 bits indicate packet type)
    // 110b_bbbb = SOP' packet (cable)
    uint8_t sopType = (rxBuffer[0] >> 5) & 0x07;
    bool isSopPrime = (sopType == 0x06); // 110 binary = 6

#ifdef FUSB302_DEBUG
    platform->debugPrint("FUSB302: Received SOP type=0x%02X (%s)\r\n", sopType,
                         isSopPrime ? "SOP' cable" : "other");
#endif

    // Read the rest of the packet (header + data + CRC)
    // Try to read up to 30 bytes (max packet size)
    for (int i = 0; i < 30 && rxLen < rxBufferSize; i++) {
        FUSB302_ReadStatusData(platform, data, FUSB302_REG_STATUS1);
        uint8_t rxEmpty = FUSB302_GetDataBit(data, FUSB302_REG_STATUS1, FUSB302_RX_EMPTY);
        if (rxEmpty) {
            break;
        }
        if (FUSB302_ReadFIFO(platform, &rxBuffer[rxLen], 1)) {
            rxLen++;
        } else {
            break;
        }
    }

    // Print received data to debug console
#ifdef FUSB302_DEBUG
    platform->debugPrint("FUSB302: EMarker data received (%d bytes):", rxLen);
    for (int i = 0; i < rxLen; i++) {
        platform->debugPrint(" %02X", rxBuffer[i]);
    }
    platform->debugPrint("\r\n");
#endif

    return true;
}

static bool ReadEMarkerData(FUSB302_Platform_t *platform, FUSB302_Data_t *data,
                            FUSB302_HostMonitoring_t *monitoring) {
    // Check that CC orientation is determined and attached type is cable
    if (monitoring->ccOrientation == FUSB302_CC_ORIENTATION_UNKNOWN ||
        (monitoring->cableType != FUSB302_CABLE_TYPE_EMARKER &&
         monitoring->cableType != FUSB302_CABLE_TYPE_EMARKER_ONLY)) {
        return false;
    }

    bool ok = true;

    // Setup switches to drive voltage to inactive CC, BMC transmitter and CRC
    FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_MEAS_CC1, 0);
    FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_MEAS_CC2, 0);

    FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES1, FUSB302_POWERROLE, 1); // 1: Source if SOP
    FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES1, FUSB302_DATAROLE, 1);  // 1: SRC
    FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES1, FUSB302_AUTOCRC, 1);   // 1: auto GoodCRC ack

    if (monitoring->ccOrientation == FUSB302_CC_ORIENTATION_CC1) {
        FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_PU_EN1, 1);
        FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_PU_EN2, 0);
        FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_VCONN_CC1, 0);
        FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_VCONN_CC2, 1);

        FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES1, FUSB302_TXCC1, 1);
        FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES1, FUSB302_TXCC2, 0);
    } else if (monitoring->ccOrientation == FUSB302_CC_ORIENTATION_CC2) {
        FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_PU_EN1, 0);
        FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_PU_EN2, 1);
        FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_VCONN_CC1, 1);
        FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES0, FUSB302_VCONN_CC2, 0);

        FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES1, FUSB302_TXCC1, 0);
        FUSB302_SetDataBit(data, FUSB302_REG_SWITCHES1, FUSB302_TXCC2, 1);
    }

    ok &= FUSB302_WriteControlData(platform, data, FUSB302_REG_SWITCHES0);
    ok &= FUSB302_WriteControlData(platform, data, FUSB302_REG_SWITCHES1);

    // Enable internal oscillator for BMC/PD communication (required for TX/RX)
    // The receiver auto-enables it on activity, but for transmission we need it enabled
    FUSB302_SetDataBit(data, FUSB302_REG_POWER, FUSB302_PWR_INT_OSC, 1);
    ok &= FUSB302_WriteControlData(platform, data, FUSB302_REG_POWER);

    // Wait for VCONN to stabilize
    platform->delayUs(20000);

    // Flush TX FIFO before sending
    FUSB302_SetDataBit(data, FUSB302_REG_CONTROL0, FUSB302_TX_FLUSH, 1);
    ok &= FUSB302_WriteControlData(platform, data, FUSB302_REG_CONTROL0);

    // Enable SOP' (SOP prime) packet detection for cable communication and flush RX FIFO
    FUSB302_SetDataBit(data, FUSB302_REG_CONTROL1, FUSB302_ENSOP1, 1);
    FUSB302_SetDataBit(data, FUSB302_REG_CONTROL1, FUSB302_RX_FLUSH, 1);
    ok &= FUSB302_WriteControlData(platform, data, FUSB302_REG_CONTROL1);

    // Enable I_CRC_CHK and I_ACTIVITY interrupts
    // I_ACTIVITY fires when BMC activity is detected (helps detect cable response)
    // I_CRC_CHK fires when a valid CRC packet is received
    FUSB302_SetDataBit(data, FUSB302_REG_MASK, FUSB302_M_CRC_CHK, 0);  // 0: Do not mask
    FUSB302_SetDataBit(data, FUSB302_REG_MASK, FUSB302_M_ACTIVITY, 0); // 0: Do not mask
    ok &= FUSB302_WriteControlData(platform, data, FUSB302_REG_MASK);

    // Read interrupt register to clear any pending interrupts
    ok &= FUSB302_ReadStatusData(platform, data, FUSB302_REG_INTERRUPT);

    // Send discovery identity packet
    uint8_t txBuffer[16];
    ok &= SendDiscoveryIdentity(platform, data, monitoring, txBuffer, sizeof(txBuffer));

#ifdef FUSB302_DEBUG
    platform->debugPrint("FUSB302: EMarker Discover Identity sent (SOP')\r\n");
#endif

    // Wait for cable response
    // tTransmit max is 195us, cable should respond within tReceive (0.9-1.1ms)
    // We'll poll for I_ACTIVITY first (BMC activity detected), then I_CRC_CHK
    bool responseReceived = false;
    uint8_t rxBuffer[80]; // Max FIFO size

    for (int retry = 0; retry < 20; retry++) {
        platform->delayUs(500); // 500us per retry = 10ms total max

        ok &= FUSB302_ReadStatusData(platform, data, FUSB302_REG_INTERRUPT);
        uint8_t i_activity = FUSB302_GetDataBit(data, FUSB302_REG_INTERRUPT, FUSB302_I_ACTIVITY);
        uint8_t i_crc_chk = FUSB302_GetDataBit(data, FUSB302_REG_INTERRUPT, FUSB302_I_CRC_CHK);

        if (i_activity) {
#ifdef FUSB302_DEBUG
            platform->debugPrint("FUSB302: BMC activity detected\r\n");
#endif
        }

        if (i_crc_chk) {
#ifdef FUSB302_DEBUG
            platform->debugPrint("FUSB302: Valid CRC packet received\r\n");
#endif
            // Check for received packet
            ok &= FUSB302_ReadStatusData(platform, data, FUSB302_REG_STATUS1);
            uint8_t rxEmpty = FUSB302_GetDataBit(data, FUSB302_REG_STATUS1, FUSB302_RX_EMPTY);

            if (!rxEmpty) {
                ok &= ReadDiscoveryIdentity(platform, data, monitoring, rxBuffer, sizeof(rxBuffer));
                responseReceived = true;
                break;
            }
        }
    }

    if (!responseReceived) {
#ifdef FUSB302_DEBUG
        platform->debugPrint("FUSB302: No EMarker response received\r\n");
#endif
    }

    // Disable SOP' detection after reading
    FUSB302_SetDataBit(data, FUSB302_REG_CONTROL1, FUSB302_ENSOP1, 0);
    FUSB302_WriteControlData(platform, data, FUSB302_REG_CONTROL1);

    // Mask interrupts again (restore original state)
    FUSB302_SetDataBit(data, FUSB302_REG_MASK, FUSB302_M_CRC_CHK, 1);
    FUSB302_SetDataBit(data, FUSB302_REG_MASK, FUSB302_M_ACTIVITY, 1);
    FUSB302_WriteControlData(platform, data, FUSB302_REG_MASK);

    return ok;
}
#endif

bool FUSB302_SetupHostMonitoring(FUSB302_Platform_t *platform, FUSB302_Data_t *data,
                                 FUSB302_HostCurrentMode_t hostCurrentMode,
                                 FUSB302_HostMonitoring_t *monitoring) {
    // Reset FUSB302
    if (!FUSB302_Reset(platform, data)) {
        return false;
    }

    platform->delayUs(10000);

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
        // 6'b10_0101 (1.596 V)
        uint8_t mdacValue = 0b100101;
        FUSB302_SetDataValue(data, FUSB302_REG_MEASURE, FUSB302_MDAC_BITS, FUSB302_MDAC_OFFSET,
                             mdacValue);
    } else {
        // 6'b11_1101 (2.604 V)
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
    FUSB302_SetDataValue(data, FUSB302_REG_MASK, 0xFF, 0,
                         ~(FUSB302_M_COMP_CHNG | FUSB302_M_BC_LVL));
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
    monitoring->hostCurrentMode = hostCurrentMode;
    monitoring->ccOrientation = FUSB302_CC_ORIENTATION_UNKNOWN;
    monitoring->cableType = FUSB302_CABLE_TYPE_UNKNOWN;

#ifdef FUSB302_DEBUG
    platform->debugPrint("FUSB302: Host monitoring started\r\n");
#endif

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

            monitoring->state = FUSB302_HOST_STATE_DETACHED;
            monitoring->ccOrientation = FUSB302_CC_ORIENTATION_UNKNOWN;
            monitoring->cableType = FUSB302_CABLE_TYPE_NONE;

#ifdef FUSB302_DEBUG
            platform->debugPrint("FUSB302: Host monitoring state changed to DETACHED\r\n");
#endif

        } else {
            // 0: Measured CC* input is lower than reference level driven from the MDAC.

            monitoring->state = FUSB302_HOST_STATE_ATTACHED;

#ifdef FUSB302_DEBUG
            platform->debugPrint("FUSB302: Host monitoring state changed to ATTACHED\r\n");
#endif

            bool ok = true;

            // Backup switches and interrupt mask register
            uint8_t switches0Value = *FUSB302_GetRegPtr(data, FUSB302_REG_SWITCHES0);
            uint8_t switches1Value = *FUSB302_GetRegPtr(data, FUSB302_REG_SWITCHES1);
            uint8_t maskValue = *FUSB302_GetRegPtr(data, FUSB302_REG_MASK);

            // Discover CC orientation and cable type
            ok &= DiscoverCCOrientation(platform, data, monitoring);

#ifdef EXPERIMENTAL_EMARKER_READ
            // If cable detected, read EMarker data
            if (monitoring->cableType == FUSB302_CABLE_TYPE_EMARKER ||
                monitoring->cableType == FUSB302_CABLE_TYPE_EMARKER_ONLY) {
                ReadEMarkerData(platform, data, monitoring);
            }
#endif

            // Restore registers
            *FUSB302_GetRegPtr(data, FUSB302_REG_SWITCHES1) = switches1Value;
            ok &= FUSB302_WriteControlData(platform, data, FUSB302_REG_SWITCHES1);

            *FUSB302_GetRegPtr(data, FUSB302_REG_SWITCHES0) = switches0Value;
            ok &= FUSB302_WriteControlData(platform, data, FUSB302_REG_SWITCHES0);

            *FUSB302_GetRegPtr(data, FUSB302_REG_MASK) = maskValue;
            ok &= FUSB302_WriteControlData(platform, data, FUSB302_REG_MASK);

            platform->delayUs(1000);

            // Read interrupt status register to clear interrupts
            ok &= FUSB302_ReadStatusData(platform, data, FUSB302_REG_INTERRUPT);

            // Check error
            if (!ok) {
                return false;
            }
        }
    }

    return true;
}
