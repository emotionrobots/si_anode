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


int util_msleep(long ms);
bool util_is_numeric(char *str);
double util_clamp(double x, double lo, double hi);
double util_temp_adj(double k_ref, double Ea, double T_C, double Tref_C);
double util_temp_unadj(double k_val, double Ea, double T_C, double Tref_C);

#endif // __UTIL_H__
