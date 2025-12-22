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
   flash_params_t *params;
   ecm_t *ecm;

   double I_quit; 
   double V_chg;
   double I_chg;
}
fgic_t;

int fgic_init(fgic_t *fgic, flash_params_t *p, double T0_C);
int fgic_get_cccv(fgic_t *fgic, double *cc, double *cv);
int fgic_update(fgic_t *fgic, double I, double T, double t, double dt);
void fgic_cleanup(fgic_t *fgic);


#endif // __FGIC_H__
