#ifndef SOC_OCV_LOOKUP_H
#define SOC_OCV_LOOKUP_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Interpolated SOC lookup from OCV using a 21-entry (N=21) SOC/OCV table.
 *
 * charging_state:
 *   -1 = charging  (SOC tends to increase) -> prefer soc_new >= soc_prev
 *    0 = rest      -> prefer closest to soc_prev
 *    1 = discharging (SOC tends to decrease) -> prefer soc_new <= soc_prev
 *
 * Returns: best-estimate SOC in [soc_table[0], soc_table[N-1]] (typically [0,1]).
 */
double soc_from_ocv_best(
    double ocv,
    double soc_prev,
    int charging_state,
    const double *soc_table,
    const double *ocv_table,
    size_t n
);

#ifdef __cplusplus
}
#endif

#endif /* SOC_OCV_LOOKUP_H */

