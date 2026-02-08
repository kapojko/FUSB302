#ifndef FUSB302_HOST_H
#define FUSB302_HOST_H

#include "FUSB302.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum FUSB302_HostState {
    FUSB302_HOST_STATE_INIT,
    FUSB302_HOST_STATE_DETACHED,
    FUSB302_HOST_STATE_ATTACHED
} FUSB302_HostState_t;

typedef struct FUSB302_HostMonitoring {
    FUSB302_HostState_t state;
    FUSB302_HostCurrentMode_t hostCurrentMode;
    FUSB302_CC_Orientation_t ccOrientation;
    FUSB302_CableType_t cableType;
} FUSB302_HostMonitoring_t;

bool FUSB302_SetupHostMonitoring(FUSB302_Platform_t *platform, FUSB302_Data_t *data,
                                 FUSB302_HostCurrentMode_t hostCurrentMode,
                                 FUSB302_HostMonitoring_t *monitoring);
bool FUSB302_UpdateHostMonitoring(FUSB302_Platform_t *platform, FUSB302_Data_t *data,
                                  FUSB302_HostMonitoring_t *monitoring);

#ifdef __cplusplus
}
#endif

#endif // FUSB302_HOST_H
