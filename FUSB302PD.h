#ifndef FUSB302_PD_H
#define FUSB302_PD_H

#include <stdint.h>
#include "FUSB302.h"

typedef enum FUSB302_SOP {
    FUSB302_SOP,
    FUSB302_SOP_PRIME,
    FUSB302_SOP_DOUBLE_PRIME,
    FUSB302_SOP_OTHER,
} FUSB302_SOP_t;

bool FUSB302_SendPacket(FUSB302_Platform_t *platform, FUSB302_Data_t *data, FUSB302_SOP_t sop,
                        uint8_t *packedData, int packedDataLen, uint8_t *txBuffer,
                        int txBufferSize);

bool FUSB302_ReadPacket(FUSB302_Platform_t *platform, FUSB302_Data_t *data, uint8_t *rxBuffer,
                        int rxBufferSize, FUSB302_SOP_t *sop);

bool FUSB302_HostCableDiscoverIdentity(FUSB302_Platform_t *platform, FUSB302_Data_t *data,
                                       FUSB302_CC_Orientation_t ccOrientation, bool checkOnly,
                                       bool *emarkerPresent);
#endif // FUSB302_PD_H
