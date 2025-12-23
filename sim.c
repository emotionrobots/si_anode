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

#include "globals.h"
#include "flash_params.h"
#include "sim.h"


extern flash_params_t g_flash_params;


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void timer_callback(void *usr_arg)
 *
 *  @brief	Timer callback for REALTIME runs
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
#ifdef REALTIME 
static
void timer_callback(void *usr_arg)
{
   sim_t *sim = (sim_t *)usr_arg;
   sim_update(sim);
}
#endif



/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int sim_init(sim_t *sim)
 *
 *  @brief	Init simulation object
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int sim_init(sim_t *sim)
{
   sim->t = 0.0; 
   sim->dt = DT;
     
   sim->batt = (batt_t *)malloc(sizeof(batt_t));
   if (sim->batt == NULL) goto err_ret;
   batt_init(sim->batt, &g_flash_params, TEMP_0);

   sim->fgic = (fgic_t *)malloc(sizeof(fgic_t));
   if (sim->fgic == NULL) goto err_ret;
   fgic_init(sim->fgic, &g_flash_params, TEMP_0);

   sim->system = (system_t *)malloc(sizeof(system_t));
   if (sim->system == NULL) goto err_ret;
   system_init(sim->system);
   system_connect_fgic(sim->system, sim->fgic);

#ifdef REALTIME 
   sim->tm = itimer_create(timer_callback, sim);
   if (sim->tm == NULL) goto err_ret;
   itimer_start(sim->tm, FGIC_RUN_PER_MS);
#endif
   return 0;

err_ret:
   return -1;
}


/*!
 *----------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void sim_cleanup(sim_t *sim)
 *
 *  @brief	Clean up simulation
 *
 *----------------------------------------------------------------------------------------------------------------------
 */
void sim_cleanup(sim_t *sim)
{
   if (sim->batt != NULL) free(sim->batt);
   if (sim->fgic != NULL) free(sim->fgic);
   if (sim->system != NULL) free(sim->system);
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
   bool rc = false;

#if 0
   if (sim->batt->v_batt < sim->flash_param.v_end)
      rc = true;
#endif

   return rc;
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

   // printf("t=%.2f, I_load=%.2f\n", sim->t, sim->system->I_load);

   rc = system_update(sim->system, sim->t, sim->dt);
   if (rc != 0) return -2;

#if 0
   rc = batt_update(sim->batt, sim->t, sim->dt);
   if (rc != 0) return -3; 

   rc = fgic_update(sim->fgic, sim->t, sim->dt);
   if (rc != 0) return -4;
#endif

   sim->t += sim->dt;
   return rc; 
}


