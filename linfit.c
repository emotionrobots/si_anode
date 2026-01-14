/*!
 *=====================================================================================================================
 *
 *  @file	linefit.c
 *
 *  @brief	line fit implementation
 *
 *=====================================================================================================================
 */
#include "linfit.h"
#include <math.h>
#include <float.h>


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int is_finite(double v) 
 *
 *  @brief	Return nonzero if 'v' is infinite
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
int is_finite(double v) 
{
    return isfinite(v);
}



/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		long double sum_ld(const double *a, size_t n) 
 *
 *  @brief	Safe-ish accumulator using long double to reduce roundoff on large n 
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
long double sum_ld(const double *a, size_t n) 
{
    long double s = 0.0L;
    for (size_t i = 0; i < n; i++) s += (long double)a[i];
    return s;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		const char* linfit_status_str(linfit_status_t s) 
 *
 *  @brief	Return string description of status
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
const char* linfit_status_str(linfit_status_t s) 
{
    switch (s) 
    {
        case LINFIT_OK: return "OK";
        case LINFIT_ERR_NULL: return "NULL pointer";
        case LINFIT_ERR_N_TOO_SMALL: return "n too small";
        case LINFIT_ERR_DEGENERATE_X: return "degenerate x (zero variance)";
        case LINFIT_ERR_NUMERIC: return "numeric error (non-finite / overflow)";
        default: return "unknown";
    }
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		linfit_status_t linfit_ols(const double *x, const double *y, size_t n, linfit_result_t *out)
 *
 *  @brief	Perform linear fit the protection
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
linfit_status_t linfit_ols(const double *x, const double *y, size_t n, linfit_result_t *out)
{
    if (!x || !y || !out) return LINFIT_ERR_NULL;
    if (n < 2) return LINFIT_ERR_N_TOO_SMALL;

    /* Validate inputs */
    for (size_t i = 0; i < n; i++) 
    {
        if (!is_finite(x[i]) || !is_finite(y[i])) return LINFIT_ERR_NUMERIC;
    }

    const long double sx = sum_ld(x, n);
    const long double sy = sum_ld(y, n);
    const long double inv_n = 1.0L / (long double)n;
    const long double mean_x = sx * inv_n;
    const long double mean_y = sy * inv_n;

    /* Compute centered sums: Sxx = sum((xi-mx)^2), Sxy = sum((xi-mx)(yi-my)) */
    long double Sxx = 0.0L;
    long double Sxy = 0.0L;
    long double Syy = 0.0L;

    for (size_t i = 0; i < n; i++) 
    {
        long double dx = (long double)x[i] - mean_x;
        long double dy = (long double)y[i] - mean_y;
        Sxx += dx * dx;
        Sxy += dx * dy;
        Syy += dy * dy;
    }

    /* Degenerate x: cannot determine slope */
    const long double eps = 10.0L * (long double)DBL_EPSILON;
    if (!(Sxx > eps)) return LINFIT_ERR_DEGENERATE_X;

    const long double b = Sxy / Sxx;
    const long double a = mean_y - b * mean_x;

    if (!is_finite((double)a) || !is_finite((double)b)) return LINFIT_ERR_NUMERIC;

    /* Compute SSE and R^2 */
    long double sse = 0.0L;
    for (size_t i = 0; i < n; i++) 
    {
        long double yhat = a + b * (long double)x[i];
        long double r = (long double)y[i] - yhat;
        sse += r * r;
    }

    long double r2;
    if (Syy <= eps) 
    {
        /* y is (near) constant. If sse is also ~0, define R^2=1, else 0 */
        r2 = (sse <= eps) ? 1.0L : 0.0L;
    } 
    else 
    {
        r2 = 1.0L - (sse / Syy);
        /* clamp small numeric overshoots */
        if (r2 < 0.0L) r2 = 0.0L;
        if (r2 > 1.0L) r2 = 1.0L;
    }

    /* Standard errors (only meaningful if n > 2) */
    double se_b = NAN, se_a = NAN;
    if (n > 2) 
    {
        long double sigma2 = sse / (long double)(n - 2); /* residual variance */
        if (sigma2 < 0.0L) sigma2 = 0.0L;

        long double var_b = sigma2 / Sxx;
        long double var_a = sigma2 * (inv_n + (mean_x * mean_x) / Sxx);

        se_b = (double)sqrt((double)var_b);
        se_a = (double)sqrt((double)var_a);
    }

    out->intercept = (double)a;
    out->slope = (double)b;
    out->r2 = (double)r2;
    out->sse = (double)sse;
    out->stderr_slope = se_b;
    out->stderr_intercept = se_a;

    return LINFIT_OK;
}

