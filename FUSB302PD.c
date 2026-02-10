#include "FUSB302PD.h"

// #define FUSB302_DEBUG_1

bool FUSB302_SendPacket(FUSB302_Platform_t *platform, FUSB302_Data_t *data, FUSB302_SOP_t sop,
                        uint8_t *packedData, int packedDataLen, uint8_t *txBuffer,
                        int txBufferSize) {
    uint8_t txLen = 0;

    if (sop == FUSB302_SOP_PRIME) {
        // SOP' token sequence for cable communication
        txBuffer[txLen++] = FUSB302_TOKEN_SOP1; // SOP1
        txBuffer[txLen++] = FUSB302_TOKEN_SOP1; // SOP1
        txBuffer[txLen++] = FUSB302_TOKEN_SOP3; // SOP3
        txBuffer[txLen++] = FUSB302_TOKEN_SOP3; // SOP3
    } else {
        // TODO: implement
        return false;
    }

    // PACKSYM token with 2 bytes
    // PACKSYM encoding: 0x80 | number_of_bytes (N must be 2-30)
    txBuffer[txLen++] = FUSB302_TOKEN_PACKSYM | packedDataLen;

    for (int i = 0; i < packedDataLen; i++) {
        txBuffer[txLen++] = packedData[i];
    }

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

bool FUSB302_ReadPacket(FUSB302_Platform_t *platform, FUSB302_Data_t *data, uint8_t *rxBuffer,
                        int rxBufferSize, FUSB302_SOP_t *sop) {
    bool ok = true;

    // Check for received packet
    ok &= FUSB302_ReadStatusData(platform, data, FUSB302_REG_STATUS1);
    uint8_t rxEmpty = FUSB302_GetDataBit(data, FUSB302_REG_STATUS1, FUSB302_RX_EMPTY);
    if (rxEmpty) {
        *sop = FUSB302_SOP_OTHER;
        return false;
    }

    // Read response from RX FIFO
    uint8_t rxLen = 0;

    // Read first byte to determine SOP type
    ok &= FUSB302_ReadFIFO(platform, &rxBuffer[rxLen], 1);
    rxLen++;

    // Check SOP type (upper 3 bits indicate packet type)
    uint8_t sopType = rxBuffer[0] & FUSB302_RXTOKEN_BITMASK;
    if (sopType == FUSB302_RXTOKEN_SOP) {
        *sop = FUSB302_SOP;
    } else if (sopType == FUSB302_RXTOKEN_SOP1) {
        *sop = FUSB302_SOP_PRIME;
    } else if (sopType == FUSB302_RXTOKEN_SOP2) {
        *sop = FUSB302_SOP_DOUBLE_PRIME;
    } else {
        *sop = FUSB302_SOP_OTHER;
    }

    // Read the rest of the packet (header + data + CRC)
    while (rxLen < rxBufferSize) {
        ok &= FUSB302_ReadStatusData(platform, data, FUSB302_REG_STATUS1);
        rxEmpty = FUSB302_GetDataBit(data, FUSB302_REG_STATUS1, FUSB302_RX_EMPTY);
        if (rxEmpty) {
            break;
        }
        ok &= FUSB302_ReadFIFO(platform, &rxBuffer[rxLen], 1);
        rxLen++;
    }

    // Print received data to debug console
#ifdef FUSB302_DEBUG
    platform->debugPrint(
        "FUSB302: EMarker data received (SOP type=0x%02X sop=%d %d bytes):", sopType, *sop, rxLen);
    for (int i = 0; i < rxLen; i++) {
        platform->debugPrint(" %02X", rxBuffer[i]);
    }
    platform->debugPrint("\r\n");
#endif

    return ok;
}

bool FUSB302_HostCableDiscoverIdentity(FUSB302_Platform_t *platform, FUSB302_Data_t *data,
                                       FUSB302_CC_Orientation_t ccOrientation, bool checkOnly,
                                       bool *emarkerPresent) {
    bool ok = true;

    // Flush TX FIFO before sending
    FUSB302_SetDataBit(data, FUSB302_REG_CONTROL0, FUSB302_TX_FLUSH, 1);
    ok &= FUSB302_WriteControlData(platform, data, FUSB302_REG_CONTROL0);
    FUSB302_SetDataBit(data, FUSB302_REG_CONTROL0, FUSB302_TX_FLUSH, 0);

    // Flush RX FIFO
    FUSB302_SetDataBit(data, FUSB302_REG_CONTROL1, FUSB302_RX_FLUSH, 1);
    ok &= FUSB302_WriteControlData(platform, data, FUSB302_REG_CONTROL1);
    FUSB302_SetDataBit(data, FUSB302_REG_CONTROL1, FUSB302_RX_FLUSH, 0);

    // Read interrupt register to clear any pending interrupts
    ok &= FUSB302_ReadStatusData(platform, data, FUSB302_REG_INTERRUPT);

    // Build packet
    uint8_t packedData[6];
    int packedDataLen = 0;

    // PD Header: Message Type 1 (Discover Identity), Port Data Role=Source(1), Port Power
    packedData[packedDataLen++] = 0x6F; // Corrected: DataRole=1 (DFP/Source)
    packedData[packedDataLen++] = 0x11; // Corrected: NumDataObj=1, PowerRole=1 (Source)

    // VDM Header (little-endian):
    packedData[packedDataLen++] = 0x01; // Command = 1 (Discover Identity)
    packedData[packedDataLen++] = 0x80; // VDMType=1, Version=0, ObjPos=0, CmdType=0
    packedData[packedDataLen++] = 0x00; // SVID low byte
    packedData[packedDataLen++] = 0xFF; // SVID high byte

    // Send discovery identity packet
    uint8_t txBuffer[16];
    ok &= FUSB302_SendPacket(platform, data, FUSB302_SOP_PRIME, packedData, packedDataLen, txBuffer,
                             sizeof(txBuffer));

#ifdef FUSB302_DEBUG_1
    platform->debugPrint("FUSB302: EMarker Discover Identity sent (SOP')\r\n");
#endif

    // Wait for cable response
    // tTransmit max is 195us, cable should respond within tReceive (0.9-1.1ms)
    bool responseReceived = false;
    uint8_t rxBuffer[80]; // Max FIFO size

    for (int retry = 0; retry < 20; retry++) {
        platform->delayUs(500); // 10ms total max

        ok &= FUSB302_ReadStatusData(platform, data, FUSB302_REG_INTERRUPT);
        uint8_t i_crc_chk = FUSB302_GetDataBit(data, FUSB302_REG_INTERRUPT, FUSB302_I_CRC_CHK);

        if (i_crc_chk) {
#ifdef FUSB302_DEBUG_1
            platform->debugPrint("FUSB302: Valid CRC packet received\r\n");
#endif

            // Check for received packet
            ok &= FUSB302_ReadStatusData(platform, data, FUSB302_REG_STATUS1);
            uint8_t rxEmpty = FUSB302_GetDataBit(data, FUSB302_REG_STATUS1, FUSB302_RX_EMPTY);
            uint8_t rxSop1 = FUSB302_GetDataBit(data, FUSB302_REG_STATUS1, FUSB302_RXSOP1);

            if (!rxEmpty) {
                if (checkOnly) {
                    // Flush RX FIFO
                    FUSB302_SetDataBit(data, FUSB302_REG_CONTROL1, FUSB302_RX_FLUSH, 1);
                    ok &= FUSB302_WriteControlData(platform, data, FUSB302_REG_CONTROL1);
                    FUSB302_SetDataBit(data, FUSB302_REG_CONTROL1, FUSB302_RX_FLUSH, 0);

                    if (rxSop1) {
                        responseReceived = true;
                        break;
                    }
                } else {
                    // Read packet
                    FUSB302_SOP_t sop;
                    ok &= FUSB302_ReadPacket(platform, data, rxBuffer, sizeof(rxBuffer), &sop);

                    if (sop == FUSB302_SOP_PRIME) {
                        responseReceived = true;
                        break;
                    }
                }
            }
        }
    }

    if (!responseReceived) {
#ifdef FUSB302_DEBUG
        platform->debugPrint("FUSB302: No EMarker response received\r\n");
#endif
    }

    *emarkerPresent = responseReceived;

    return ok;
}
