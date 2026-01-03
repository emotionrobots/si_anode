/*!
 *=====================================================================================================================
 *
 *  @file		fgic.h
 *
 *  @brief		FGIC object header 
 *
 *=====================================================================================================================
 */
#ifndef __FGIC_H__
#define __FGIC_H__

#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>


#include "flash_params.h"
#include "ecm.h"
#include "batt.h"


typedef struct {
   batt_t *batt;			// battery model
   flash_params_t *params;		// Pointer to flash params
   ecm_t *ecm;				// ECM model
   int period;				// FGIC run period in ms
   double I_quit;			// quit current 
   double V_chg;			// desired charging voltage
   double I_chg;			// desired charging current
   double V_noise;			// voltage measurement noise amplitude
   double I_noise;			// current measurement noise amplitude
   double T_noise;			// temperature measurement noise amplitude 
   double V_offset;			// voltage measurement offset
   double I_offset;			// current measurement offset 
   double T_offset;			// temperature measurement offset 
}
fgic_t;

fgic_t *fgic_create(batt_t *batt, flash_params_t *p, double T0_C);
int fgic_get_cccv(fgic_t *fgic, double *cc, double *cv);
int fgic_update(fgic_t *fgic, double t, double dt);
void fgic_destroy(fgic_t *fgic);


#endif // __FGIC_H__
