/*!
 *=====================================================================================================================
 *
 *  @file		system.h
 *
 *  @brief		System object header 
 *
 *=====================================================================================================================
 */
#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <inttypes.h>
#include <time.h>
#include <errno.h>

#include "fgic.h"

typedef struct {
   int load_type;

   double I; 			
   double V; 			
   double V_chg;
   double I_chg;

   double per;
   double dutycycle;
   double I_on; 
   double I_off;
   double t_start;

   fgic_t *fgic;
}
system_t;


system_t *system_create();
int system_get_cccv(system_t *sys);
int system_update(system_t *sys, double t, double dt);
void system_destroy(system_t *sys);


#endif // __SYSTEM_H__
