/*!
 *=====================================================================================================================
 *
 *  @file		sim.c
 *
 *  @brief		Simulation object implementation 
 *
 *=====================================================================================================================
 */
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <inttypes.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>

#include "globals.h"
#include "flash_params.h"
#include "sim.h"


extern flash_params_t g_flash_params;


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int params_init(sim_t *sim)
 *
 *  @brief	string-enabled parameter init
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
int params_init(sim_t *sim)
{
   int i=0;
   sim->params[i].name = "t";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->t;

   sim->params[i].name = "dt";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->dt;

   sim->params_sz = i;
   return i;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void timer_callback(void *usr_arg)
 *
 *  @brief	Timer callback for REALTIME runs
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
void timer_callback(void *usr_arg)
{
   sim_t *sim = (sim_t *)usr_arg;
   sim_update(sim);
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void sim_set_pause(sim_t *sim, bool do_pause)
 *
 *  @brief	Set/unset pause for the simulation
 *
 *  @param	sim:		simulation pointer
 *  @param	do_pause	false: unpause; true: pause
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void sim_set_pause(sim_t *sim, bool do_pause)
{
   LOCK(&sim->mtx); 
   sim->pause = do_pause; 
   UNLOCK(&sim->mtx); 
}

/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn         bool sim_get_pause(sim_t *sim)
 *
 *  @brief      Get simulatlion pause state 
 *
 *  @param      sim:            simulation pointer
 *
 *  @return	true if paused; false otherwise
 *
 *  @note	mutex protected
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
bool sim_get_pause(sim_t *sim)
{
   bool paused = false;
   LOCK(&sim->mtx);
   paused = sim->pause;
   UNLOCK(&sim->mtx);

   return paused;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void *sim_loop(void *arg)
 *
 *  @brief	Simulation thread loop
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
void *sim_loop(void *arg)
{
   if (arg==NULL) return NULL;
   sim_t *sim = (sim_t *)arg;

   bool done = sim_check_exit(sim);  
   bool pause = sim->pause;
   while (!done)
   {
      /* check pause */
      LOCK(&sim->mtx); 
      sim->pause |= (sim->t >= sim->t_end);
      pause = sim->pause; 
      UNLOCK(&sim->mtx); 
      while (pause)
      {
         LOCK(&sim->mtx); 
         pause = sim->pause; 
         UNLOCK(&sim->mtx); 
         sched_yield();
      }

      /* update sim */
      LOCK(&sim->mtx); 
      sim_update(sim);  
      done = sim_check_exit(sim);
      UNLOCK(&sim->mtx); 
      sched_yield();
   }

   return NULL;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		sim_t *sim_create(double t0, double dt, double temp0)
 *
 *  @brief	Create and initialize simulation object
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
sim_t *sim_create(double t0, double dt, double temp0)
{
   sim_t *sim = calloc(1, sizeof(sim_t));
   if (sim == NULL) return NULL;

   sim->t = t0; 
   sim->dt = dt;

   sim->realtime = false;
   sim->done = false;
   sim->pause = true;

   sim->batt = batt_create(&g_flash_params, temp0);
   if (sim->batt == NULL) goto err_ret;

   sim->fgic = fgic_create(&g_flash_params, temp0);
   if (sim->fgic == NULL) goto err_ret;

   sim->system = (system_t *)system_create(sim->fgic);
   if (sim->system == NULL) goto err_ret;

   if (pthread_mutex_init(&sim->mtx, NULL) != 0)
      goto err_ret;

   sim->thread = (pthread_t *)calloc(1, sizeof(pthread_t));
   if (sim->thread == NULL) goto err_ret;

   sim->tm = itimer_create(timer_callback, sim);
   if (sim->tm == NULL) goto err_ret;

   params_init(sim);

   return sim;

err_ret:
   if (sim != NULL) sim_destroy(sim);
   return NULL;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int sim_start(sim_t *sim)
 *
 *  @brief	Start simulation
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int sim_start(sim_t *sim)
{
   if (sim == NULL) return -1;

   if (sim->realtime)
      return itimer_start(sim->tm, sim->fgic->period);
   else 
      return pthread_create(sim->thread, NULL, sim_loop, sim);
}


/*!
 *----------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void sim_destroy(sim_t *sim)
 *
 *  @brief	Clean up simulation
 *
 *----------------------------------------------------------------------------------------------------------------------
 */
void sim_destroy(sim_t *sim)
{
   if (sim->tm != NULL) itimer_destroy(sim->tm);
   if (sim->thread != NULL) free(sim->thread);
   if (sim->system != NULL) system_destroy(sim->system);
   if (sim->fgic != NULL) fgic_destroy(sim->fgic);
   if (sim->batt != NULL) batt_destroy(sim->batt);
}



/*!
 *----------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		bool sim_check_exit(sim_t *sim)
 *
 *  @brief	Check if simulation should exit
 *
 *----------------------------------------------------------------------------------------------------------------------
 */
bool sim_check_exit(sim_t *sim)
{
   bool do_exit = false;
  
   do_exit |= sim->done; 
#if 0
   if (sim->batt->v_batt < sim->flash_param.v_end)
      rc = true;
#endif

   return do_exit;
}


/*!
 *----------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int sim_update(sim_t *sim)
 *
 *  @brief	Perform simulation update one time step
 *
 *----------------------------------------------------------------------------------------------------------------------
 */
int sim_update(sim_t *sim)
{
   int rc = 0;

   if (sim == NULL) return -1;

   //printf("t=%.2f, I_load=%.2f\n", sim->t, sim->system->I_load);

   rc = system_update(sim->system, sim->t, sim->dt);
   if (rc != 0) return -2;

   rc = batt_update(sim->batt, sim->system->I_load, sim->T_amb_C, sim->t, sim->dt);
   if (rc != 0) return -3; 

#if 0
   rc = fgic_update(sim->fgic, sim->t, sim->dt);
   if (rc != 0) return -4;
#endif

   sim->t += sim->dt;
   return rc; 
}


