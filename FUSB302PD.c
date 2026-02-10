#include "FUSB302PD.h"

#define GET_BITS(val, hi, lo) (((val) >> (lo)) & ((1u << ((hi) - (lo) + 1)) - 1))

// #define FUSB302_DEBUG_1

static bool ParseDiscoverIdentityReply(const uint8_t *pkt, int pktLen, FUSB302_PDIdentity_t *id) {
    if (pktLen < 6)
        return false;

    // --- PD header ---
    uint16_t header = pkt[0] | (pkt[1] << 8);
    int numObj = (header >> 12) & 0x7;

    // Must be at least VDM header + ID Header VDO
    if (numObj < 2)
        return false;

    // --- VDM header (Obj 0) ---
    uint32_t vdm =
        (uint32_t)pkt[2] | (uint32_t)pkt[3] << 8 | (uint32_t)pkt[4] << 16 | (uint32_t)pkt[5] << 24;

    uint16_t svid = GET_BITS(vdm, 31, 16);
    uint8_t cmd = GET_BITS(vdm, 4, 0);
    uint8_t cmdt = GET_BITS(vdm, 7, 6);

    // Must be PD SID + Discover Identity ACK
    if (svid != 0xFF00 || cmd != 1 || cmdt != 1)
        return false;

    // --- ID Header VDO (Obj 1) ---
    uint32_t idh =
        (uint32_t)pkt[6] | (uint32_t)pkt[7] << 8 | (uint32_t)pkt[8] << 16 | (uint32_t)pkt[9] << 24;

    uint16_t vid = GET_BITS(idh, 31, 16);

    id->vid = vid;

    // TODO: parse objects

    return true;
}

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

bool FUSB302_ReadBuffer(FUSB302_Platform_t *platform, FUSB302_Data_t *data, uint8_t *rxBuffer,
                        int rxBufferSize, int *rxLen) {
    bool ok = true;

    // Read response from RX FIFO
    *rxLen = 0;

    // Read the rest of the packet (header + data + CRC)
    while (*rxLen < rxBufferSize) {
        ok &= FUSB302_ReadStatusData(platform, data, FUSB302_REG_STATUS1);
        uint8_t rxEmpty = FUSB302_GetDataBit(data, FUSB302_REG_STATUS1, FUSB302_RX_EMPTY);
        if (rxEmpty) {
            break;
        }
        ok &= FUSB302_ReadFIFO(platform, &rxBuffer[*rxLen], 1);
        (*rxLen)++;
    }

    // Print received data to debug console
#ifdef FUSB302_DEBUG_1
    platform->debugPrint("FUSB302: PD data received (%d bytes):", *rxLen);
    for (int i = 0; i < *rxLen; i++) {
        platform->debugPrint(" %02X", rxBuffer[i]);
    }
    platform->debugPrint("\r\n");
#endif

    return ok;
}

bool FUSB302_ExtractPacket(const uint8_t *rxBuffer, int rxBufferLen, int rxBufferStart,
                           int *packetStart, int *packetLen, FUSB302_SOP_t *packetSop) {
    for (int i = rxBufferStart; i < rxBufferLen; i++) {
        // 1. Find SOP token
        // Check SOP type (upper 3 bits indicate packet type)
        uint8_t sopType = rxBuffer[i] & FUSB302_RXTOKEN_BITMASK;
        if (sopType == FUSB302_RXTOKEN_SOP) {
            *packetSop = FUSB302_SOP;
        } else if (sopType == FUSB302_RXTOKEN_SOP1) {
            *packetSop = FUSB302_SOP_PRIME;
        } else if (sopType == FUSB302_RXTOKEN_SOP2) {
            *packetSop = FUSB302_SOP_DOUBLE_PRIME;
        } else {
            continue;
        }
        // Need at least SOP + PD header
        if (i + 3 > rxBufferLen)
            return false;

        int idx = i + 1;

        // 2. Read PD header (little-endian)
        uint16_t header = ((uint16_t)rxBuffer[idx + 1] << 8) | (uint16_t)rxBuffer[idx + 0];

        // Number of Data Objects = bits 14..12
        int numDataObjects = (header >> 12) & 0x7;

        // 3. Calculate total message size
        int pdBytes = 2 + numDataObjects * 4 + 4; // header + data objs + CRC
        int totalBytes = 1 + pdBytes;             // + SOP token

        // 4. Check if buffer contains full packet
        if (i + totalBytes > rxBufferLen)
            return false;

        *packetStart = i + 1; // Skip SOP token
        *packetLen = totalBytes - 1;
        return true;
    }

    return false;
}

bool FUSB302_HostCableDiscoverIdentity(FUSB302_Platform_t *platform, FUSB302_Data_t *data,
                                       FUSB302_CC_Orientation_t ccOrientation, bool checkOnly,
                                       bool *emarkerPresent, FUSB302_PDIdentity_t *identity) {
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

    /*
     * PD Header (16-bit, little-endian)
     *
     * Message Type   = 0xF (Vendor Defined)
     * Num Data Obj   = 1
     * Power Role     = Source (1)
     * Data Role      = DFP (1)
     * Spec Revision  = 2.0 (01b)
     *
     * Header = 0x12EF → bytes EF 12
     */
    packedData[packedDataLen++] = 0xEF; // PD Header LSB
    packedData[packedDataLen++] = 0x12; // PD Header MSB

    /*
     * VDM Header (32-bit, little-endian)
     *
     * SVID           = 0xFF00 (PD SID)
     * VDM Type       = Structured (1)
     * Version        = 1 (PD 2.0)
     * Obj Position   = 0
     * Command Type   = Initiator (0)
     * Command        = Discover Identity (1)
     *
     * VDM = 0xFF008001 → bytes 01 80 00 FF
     */
    packedData[packedDataLen++] = 0x01; // Command = Discover Identity
    packedData[packedDataLen++] = 0x80; // Structured VDM, ver=1
    packedData[packedDataLen++] = 0x00; // SVID LSB
    packedData[packedDataLen++] = 0xFF; // SVID MSB

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
                    // Read buffer
                    int rxLen = 0;
                    ok &= FUSB302_ReadBuffer(platform, data, rxBuffer, sizeof(rxBuffer), &rxLen);

                    if (rxLen == 0) {
                        continue;
                    }

                    // Extract packets
                    int rxStart = 0;
                    while (rxStart < rxLen) {
                        int packetStart = rxStart, packetLen = 0;
                        FUSB302_SOP_t packetSop;
                        if (!FUSB302_ExtractPacket(rxBuffer, rxLen, rxStart, &packetStart,
                                                   &packetLen, &packetSop)) {
                            break;
                        }

#ifdef FUSB302_DEBUG_1
                        platform->debugPrint("Packet received: SOP=%d, start=%d, LEN=%d\r\n",
                                             packetSop, packetStart, packetLen);
#endif

                        // Try to parse identity reply
                        if (packetSop == FUSB302_SOP_PRIME) {
                            if (ParseDiscoverIdentityReply(&rxBuffer[packetStart], packetLen,
                                                           identity)) {
#ifdef FUSB302_DEBUG
                                platform->debugPrint("FUSB302: Identity reply VID=%04X\r\n",
                                                     identity->vid);
#endif

                                responseReceived = true;
                                break;
                            }
                        }

                        rxStart = packetStart + packetLen;
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
