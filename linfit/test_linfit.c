#include "linfit.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>

static int nearly_equal(double a, double b, double rel_tol, double abs_tol) {
    double diff = fabs(a - b);
    if (diff <= abs_tol) return 1;
    double denom = fmax(fabs(a), fabs(b));
    if (denom == 0.0) return diff <= abs_tol;
    return diff / denom <= rel_tol;
}

static void test_perfect_line(void) {
    /* y = 2 + 3x */
    const double x[] = { -2, -1, 0, 1, 2, 3 };
    const double y[] = { -4, -1, 2, 5, 8, 11 };
    linfit_result_t r;

    linfit_status_t s = linfit_ols(x, y, sizeof(x)/sizeof(x[0]), &r);
    assert(s == LINFIT_OK);
    assert(nearly_equal(r.intercept, 2.0, 1e-12, 1e-12));
    assert(nearly_equal(r.slope, 3.0, 1e-12, 1e-12));
    assert(nearly_equal(r.r2, 1.0, 1e-12, 1e-12));
    assert(nearly_equal(r.sse, 0.0, 1e-12, 1e-12));
}

static void test_noisy_line(void) {
    /* Roughly y = 1 + 0.5x */
    const double x[] = { 0, 1, 2, 3, 4, 5 };
    const double y[] = { 1.1, 1.4, 2.2, 2.3, 3.1, 3.6 };
    linfit_result_t r;

    linfit_status_t s = linfit_ols(x, y, sizeof(x)/sizeof(x[0]), &r);
    assert(s == LINFIT_OK);
    assert(nearly_equal(r.intercept, 1.0, 0.25, 0.25)); /* loose check */
    assert(nearly_equal(r.slope, 0.5, 0.25, 0.25));     /* loose check */
    assert(r.r2 >= 0.0 && r.r2 <= 1.0);
    assert(r.sse >= 0.0);
}

static void test_degenerate_x(void) {
    /* All x identical -> slope undefined */
    const double x[] = { 1, 1, 1, 1 };
    const double y[] = { 0, 1, 2, 3 };
    linfit_result_t r;

    linfit_status_t s = linfit_ols(x, y, sizeof(x)/sizeof(x[0]), &r);
    assert(s == LINFIT_ERR_DEGENERATE_X);
}

static void test_n_too_small(void) {
    const double x[] = { 0 };
    const double y[] = { 0 };
    linfit_result_t r;
    linfit_status_t s = linfit_ols(x, y, 1, &r);
    assert(s == LINFIT_ERR_N_TOO_SMALL);
}

static void test_constant_y(void) {
    /* y constant: R^2 handled gracefully */
    const double x[] = { 0, 1, 2, 3 };
    const double y[] = { 2, 2, 2, 2 };
    linfit_result_t r;
    linfit_status_t s = linfit_ols(x, y, 4, &r);
    assert(s == LINFIT_OK);
    /* Any slope/intercept that predicts y=2 well is fine; check SSE ~ 0 */
    assert(nearly_equal(r.sse, 0.0, 1e-12, 1e-12));
    assert(nearly_equal(r.r2, 1.0, 1e-12, 1e-12));
}

int main(void) {
    test_perfect_line();
    test_noisy_line();
    test_degenerate_x();
    test_n_too_small();
    test_constant_y();

    printf("All linfit tests passed.\n");
    return 0;
}

