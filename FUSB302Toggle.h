#ifndef FUSB302_TOGGLE_H
#define FUSB302_TOGGLE_H

#include "FUSB302.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum FUSB302_ToggleMode {
    FUSB302_TOGGLE_MODE_MANUAL,
    FUSB302_TOGGLE_MODE_DRP,
    FUSB302_TOGGLE_MODE_SNK,
    FUSB302_TOGGLE_MODE_SRC
} FUSB302_ToggleMode_t;

typedef enum FUSB302_ToggleResult {
    FUSB302_TOGGLE_RESULT_NONE,
    FUSB302_TOGGLE_RESULT_SRC_CC1,
    FUSB302_TOGGLE_RESULT_SRC_CC2,
    FUSB302_TOGGLE_RESULT_SNK_CC1,
    FUSB302_TOGGLE_RESULT_SNK_CC2,
    FUSB302_TOGGLE_RESULT_AUDIO
} FUSB302_ToggleResult_t;

bool FUSB302_SetupToggleMode(FUSB302_Platform_t *platform, FUSB302_Data_t *data,
                           FUSB302_ToggleMode_t mode, FUSB302_HostCurrentMode_t hostCurrentMode);
bool FUSB302_GetToggleResult(FUSB302_Platform_t *platform, FUSB302_Data_t *data,
                             FUSB302_ToggleResult_t *result);

#ifdef __cplusplus
}
#endif

#endif // FUSB302_TOGGLE_H
