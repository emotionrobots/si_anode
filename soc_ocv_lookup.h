#ifndef SOC_OCV_LOOKUP_H
#define SOC_OCV_LOOKUP_H

#include <stddef.h>
#include <float.h>
#include "flash_params.h"

#ifdef __cplusplus
extern "C" {
#endif

double soc_from_ocv_best( double ocv, double soc_prev, int charging_state, flash_params_t *p);

#ifdef __cplusplus
}
#endif

#endif /* SOC_OCV_LOOKUP_H */

