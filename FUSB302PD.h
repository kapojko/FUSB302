#ifndef FUSB302_PD_H
#define FUSB302_PD_H

#include <stdint.h>
#include <stdbool.h>
#include "FUSB302.h"

typedef enum FUSB302_SOP {
    FUSB302_SOP,
    FUSB302_SOP_PRIME,
    FUSB302_SOP_DOUBLE_PRIME,
    FUSB302_SOP_OTHER,
} FUSB302_SOP_t;

typedef struct FUSB302_PDIdentity {
    uint16_t vid;
} FUSB302_PDIdentity_t;

bool FUSB302_SendPacket(FUSB302_Platform_t *platform, FUSB302_Data_t *data, FUSB302_SOP_t sop,
                        uint8_t *packedData, int packedDataLen, uint8_t *txBuffer,
                        int txBufferSize);

bool FUSB302_ReadBuffer(FUSB302_Platform_t *platform, FUSB302_Data_t *data, uint8_t *rxBuffer,
                        int rxBufferSize, int *rxLen);
bool FUSB302_ExtractPacket(const uint8_t *rxBuffer, int rxBufferLen, int rxBufferStart,
                           int *packetStart, int *packetLen, FUSB302_SOP_t *packetSop);

bool FUSB302_HostCableDiscoverIdentity(FUSB302_Platform_t *platform, FUSB302_Data_t *data,
                                       FUSB302_CC_Orientation_t ccOrientation, bool checkOnly,
                                       bool *emarkerPresent, FUSB302_PDIdentity_t *identity);
#endif // FUSB302_PD_H
