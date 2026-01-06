/*!
 *======================================================================================================================
 *
 *  @file	soc_ocv_lookup.c
 *
 *  @brief	SOC lookup from OCV table
 *
 *======================================================================================================================
 */
#include "util.h"
#include "globals.h"
#include "soc_ocv_lookup.h"


/*!
 *----------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int is_between_inclusive(double x, double a, double b)
 *
 *  @brief	Determine if 'x' is between 'a' and 'b' inclusively
 *
 *----------------------------------------------------------------------------------------------------------------------
 */
static 
int is_between_inclusive(double x, double a, double b)
{
    const double lo = (a < b) ? a : b;
    const double hi = (a < b) ? b : a;
    return (x >= lo && x <= hi);
}


/*!
 *----------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int direction_ok(double soc_candidate, double soc_prev, int charging_state)
 *
 *  @brief	Direction preference test
 *
 *----------------------------------------------------------------------------------------------------------------------
 */
static 
int direction_ok(double soc_candidate, double soc_prev, int charging_state)
{
    /* charging_state: -1 charging, 0 rest, 1 discharging */
    if (charging_state < 0)           	/* charging */
    {
        return soc_candidate >= soc_prev;
    } 
    else if (charging_state > 0)      	/* discharging */
    {
        return soc_candidate <= soc_prev;
    }
    return 1; 				/* rest accepts all */
}


/*!
 *----------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		double score_candidate(double soc_candidate, double soc_prev, int charging_state)
 *
 *  @brief	Score: smaller is better 
 *
 *----------------------------------------------------------------------------------------------------------------------
 */
static 
double score_candidate(double soc_candidate, double soc_prev, int charging_state)
{
    const double d = fabs(soc_candidate - soc_prev);

    /* Strongly prefer direction-consistent solutions when they exist */
    if (charging_state != 0 && !direction_ok(soc_candidate, soc_prev, charging_state)) {
        return d + 1e3; /* big penalty */
    }
    return d;
}


double soc_from_ocv_best(
    double ocv,
    double soc_prev,
    int charging_state,
    const double *soc_table,
    const double *ocv_table,
    size_t n
)
{
    if (!soc_table || !ocv_table || n < 2) {
        return soc_prev; /* safest fallback */
    }

    const double soc_min = soc_table[0];
    const double soc_max = soc_table[n - 1];

    /* Clamp soc_prev into table SOC range */
    soc_prev = clampd(soc_prev, soc_min, soc_max);

    /* Track best candidate */
    double best_soc = soc_prev;
    double best_score = DBL_MAX;

    /* 1) Consider all intervals where ocv lies between ocv_table[i] and ocv_table[i+1] */
    for (size_t i = 0; i + 1 < n; i++) {
        const double v0 = ocv_table[i];
        const double v1 = ocv_table[i + 1];
        const double s0 = soc_table[i];
        const double s1 = soc_table[i + 1];

        if (!is_between_inclusive(ocv, v0, v1)) {
            continue;
        }

        double cand_soc;

        if (v0 == v1) {
            /* Flat segment in OCV: any SOC in [s0,s1] is valid.
               Pick the boundary SOC that best matches (soc_prev, direction). */
            double a = s0, b = s1;
            /* Ensure a <= b */
            if (a > b) { double tmp = a; a = b; b = tmp; }

            /* First try direction-consistent boundary if applicable */
            if (charging_state < 0) {          /* charging: prefer higher */
                cand_soc = b;
            } else if (charging_state > 0) {   /* discharging: prefer lower */
                cand_soc = a;
            } else {
                /* rest: closest boundary */
                cand_soc = (fabs(a - soc_prev) <= fabs(b - soc_prev)) ? a : b;
            }

            /* But if that boundary is direction-inconsistent, try the other boundary */
            if (charging_state != 0 && !direction_ok(cand_soc, soc_prev, charging_state)) {
                const double other = (cand_soc == a) ? b : a;
                if (direction_ok(other, soc_prev, charging_state)) {
                    cand_soc = other;
                }
            }
        } else {
            /* Linear interpolation in OCV domain */
            const double t = (ocv - v0) / (v1 - v0);  /* can be [0,1] due to is_between_inclusive */
            cand_soc = s0 + t * (s1 - s0);
        }

        /* Clamp candidate to SOC bounds */
        cand_soc = clampd(cand_soc, soc_min, soc_max);

        const double sc = score_candidate(cand_soc, soc_prev, charging_state);
        if (sc < best_score) {
            best_score = sc;
            best_soc = cand_soc;
        }
    }

    /* 2) If no interval matched, clamp by nearest endpoint.
          Still apply direction preference when possible. */
    if (best_score == DBL_MAX) {
        /* Find global min/max OCV in table (since it can be non-monotonic) */
        double vmin = ocv_table[0], vmax = ocv_table[0];
        size_t imin = 0, imax = 0;
        for (size_t i = 1; i < n; i++) {
            if (ocv_table[i] < vmin) { vmin = ocv_table[i]; imin = i; }
            if (ocv_table[i] > vmax) { vmax = ocv_table[i]; imax = i; }
        }

        /* Nearest by OCV distance */
        const double dmin = fabs(ocv - vmin);
        const double dmax = fabs(ocv - vmax);

        double cand1 = soc_table[(dmin <= dmax) ? imin : imax];
        double cand2 = soc_table[(dmin <= dmax) ? imax : imin]; /* alternate */

        /* Prefer direction-consistent one if possible */
        if (charging_state != 0) {
            if (direction_ok(cand1, soc_prev, charging_state)) {
                best_soc = cand1;
            } else if (direction_ok(cand2, soc_prev, charging_state)) {
                best_soc = cand2;
            } else {
                /* neither consistent, choose closer in SOC */
                best_soc = (fabs(cand1 - soc_prev) <= fabs(cand2 - soc_prev)) ? cand1 : cand2;
            }
        } else {
            /* rest: closest in SOC */
            best_soc = (fabs(cand1 - soc_prev) <= fabs(cand2 - soc_prev)) ? cand1 : cand2;
        }

        best_soc = clampd(best_soc, soc_min, soc_max);
    }

    return best_soc;
}

