/*!
 *=====================================================================================================================
 *
 *  @file		util.h
 *
 *  @brief		Simulation utilities header 
 *
 *=====================================================================================================================
 */
#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <inttypes.h>
#include <time.h>
#include <errno.h>
#include "sim.h"


int util_msleep(long ms);
bool util_is_numeric(char *str);
double util_clamp(double x, double lo, double hi);
double util_temp_adj(double k_ref, double Ea, double T_C, double Tref_C);
double util_temp_unadj(double k_val, double Ea, double T_C, double Tref_C);
// int util_nearest_soc_idx(const double *grid, int n, double soc);
int util_update_tbl(double *tbl, double *soc_tbl, int n, double soc, double val);
char* util_get_params_type(sim_t *sim, char *name);
int util_get_params_val(sim_t *sim, char *name, double *value);
enum LOP util_strtolop(char *op);
char *util_loptostr(enum LOP lop);


#endif // __UTIL_H__
