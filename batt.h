/*!
 *=====================================================================================================================
 *
 *  @file		batt.h
 *
 *  @brief		Battery object header 
 *
 *=====================================================================================================================
 */
#ifndef __BATT_H__
#define __BATT_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>
#include <time.h>
#include <errno.h>

#include "flash_params.h"
#include "ecm.h"


typedef struct {
   flash_params_t *params;
   ecm_t *ecm;
}
batt_t;


int batt_init(batt_t *batt, flash_params_t *p, double T0_C);
int batt_update(batt_t *batt, double I, double T, double t, double dt);
int batt_cleanup(batt_t *batt);


#endif // __BATT_H__
