/*!
 *=====================================================================================================================
 *
 *  @file		sim.h
 *
 *  @brief		Simulation object header 
 *
 *=====================================================================================================================
 */
#ifndef __SIM_H__
#define __SIM_H__

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <inttypes.h>
#include <time.h>
#include <errno.h>

#include "batt.h"
#include "fgic.h"
#include "system.h"
#include "itimer.h"


typedef struct {
   itimer_t *tm;		/* simulation interval timer */ 

   batt_t *batt;     		/* battery object */
   fgic_t *fgic;     		/* fgic object */
   system_t *system;		/* system object */

   double t;			/* simulation time */
   double dt;			/* simulation step size */
   double v_batt_noise;		/* V_batt noise */
   double T_amb_C;		/* Environment temperature */
}
sim_t;


int sim_init(sim_t *sim);
bool sim_check_exit(sim_t *sim);
int sim_update(sim_t *sim);
int sim_msleep(long ms);
void sim_cleanup(sim_t *sim);


#endif // __SIM_H__
