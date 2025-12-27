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


typedef struct {
   flash_params_t *params;		// Pointer to flash params
   ecm_t *ecm;				// ECM model
   int period;				// FGIC run period in ms
   double I_quit;			// quit current 
   double V_chg;			// desired charging voltage
   double I_chg;			// desired charging current
}
fgic_t;

fgic_t *fgic_create(flash_params_t *p, double T0_C);
int fgic_get_cccv(fgic_t *fgic, double *cc, double *cv);
int fgic_update(fgic_t *fgic, double I, double T, double t, double dt);
void fgic_destroy(fgic_t *fgic);


#endif // __FGIC_H__
