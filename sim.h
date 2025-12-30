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
#include <pthread.h>

#include "batt.h"
#include "fgic.h"
#include "system.h"
#include "itimer.h"


typedef struct {
   FILE *logfp;			/* log file pointer */
   char logfn[FN_LEN];		/* log file name */
   int logi[MAX_PARAMS];	/* log data index */
   int logn;			/* num of log items */

   params_t params[MAX_PARAMS];	/* string-enabled parameters */
   int params_sz;		/* parameter sz */

   itimer_t *tm;		/* simulation interval timer */ 

   pthread_t *thread;		/* thread object */
   pthread_mutex_t mtx;		/* sim thread mutex */

   bool realtime;		/* true if run sim in wall time */
   bool done;			/* set true to exit a sim run */
   bool pause;			/* set true to pause a sim run */

   batt_t *batt;     		/* battery object */
   fgic_t *fgic;     		/* fgic object */
   system_t *system;		/* system object */

   double t;			/* simulation time */
   double dt;			/* simulation step size */
   double t_end;		/* simulation t end */
   double v_batt_noise;		/* V_batt noise */
   double T_amb_C;		/* Environment temperature */
}
sim_t;


sim_t *sim_create(double t, double dt, double temp0);
bool sim_check_exit(sim_t *sim);
int sim_update(sim_t *sim);
int sim_msleep(long ms);
int sim_start(sim_t *sim);
bool sim_get_pause(sim_t *sim);
void sim_set_pause(sim_t *sim, bool do_pause);
void sim_destroy(sim_t *sim);


#endif // __SIM_H__
