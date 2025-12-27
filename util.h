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
#include <stdbool.h>
#include <math.h>
#include <inttypes.h>
#include <time.h>
#include <errno.h>


int util_msleep(long ms);
bool util_is_numeric(char *str);

#endif // __UTIL_H__
