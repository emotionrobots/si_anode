#ifndef LINFIT_H
#define LINFIT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LINFIT_OK = 0,
    LINFIT_ERR_NULL = -1,
    LINFIT_ERR_N_TOO_SMALL = -2,
    LINFIT_ERR_DEGENERATE_X = -3,   /* var(x) ~ 0 */
    LINFIT_ERR_NUMERIC = -4         /* non-finite input or overflow */
} linfit_status_t;

typedef struct {
    double intercept;     /* a */
    double slope;         /* b */
    double r2;            /* coefficient of determination */
    double sse;           /* sum of squared errors */
    double stderr_slope;  /* standard error of slope (if n>2), else NaN */
    double stderr_intercept; /* standard error of intercept (if n>2), else NaN */
} linfit_result_t;

/*
 * Fit y = intercept + slope * x using ordinary least squares.
 *
 * Returns:
 *  - LINFIT_OK on success
 *  - error code otherwise (see enum)
 *
 * Notes:
 *  - Requires n >= 2
 *  - Fails if x has (near) zero variance (all x equal / degenerate).
 */
linfit_status_t linfit_ols(const double *x, const double *y, size_t n,
                           linfit_result_t *out);

const char* linfit_status_str(linfit_status_t s);

#ifdef __cplusplus
}
#endif

#endif /* LINFIT_H */

