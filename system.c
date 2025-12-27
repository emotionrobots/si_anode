/*!
 *=====================================================================================================================
 *
 *  @file		system.c
 *
 *  @brief		System object implementation 
 *
 *=====================================================================================================================
 */
#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include <time.h>
#include <errno.h>

#include "globals.h"
#include "system.h"

/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		double _pulse_load(double now, double t_start, double per, double dutycycle, double I_on)
 *
 *  @brief	Create a pulse load
 *
 *  @param	t:		present time
 *  @param	t_start:	pulse train origin time
 *  @param	per:		pulse period
 *  @param	dutycycle:	pulse ON dutycycle [0,1]
 *  @param	I_on:		pulse ON current
 *
 *  @return	I_load	
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
double _pulsed_load(double now, double t_start, double per, double dutycycle, double I_on)
{
   double t = now-t_start; 
   double rem = t - floor(t/per) * per;
   double ratio = rem/per;
   return (ratio < dutycycle) ? I_on : 0.0;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		system_t *system_create()
 *
 *  @brief	System creation 
 *
 *  @return 	0 if success; negative otherwise
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
system_t *system_create(fgic_t *fgic)
{
   system_t *sys = (system_t *)calloc(1, sizeof(system_t));
   if (sys==NULL) return NULL;

   sys->I_load = 0.0;
   sys->V_chg = DEFAULT_CV;
   sys->I_chg = DEFAULT_CC;
   sys->fgic = fgic;

   return sys;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int system_update(system_t *sys, double t, double dt)
 *
 *  @brief	System update.  Generally this outputs a new I_load
 *
 *  @return 	0 if success; negative otherwise
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int system_update(system_t *sys, double t, double dt)
{
   (void)dt;

   if (sys == NULL) return -1;
   
   if (t < MAX_RUN_TIME)
   {
      if (t >= 0.0 && t < 5.0)  
         sys->I_load = 0.1;
      else if (t >= 5.0 && t < 10.0)  
         sys->I_load = 0.0;
      else if (t >= 10.0 && t < 20.0)
         sys->I_load = 2.0;
      else
         sys->I_load = _pulsed_load(t, 20.0, 10.0, 0.5, 3.3);
   }
   else
   {
      sys->I_load = 0.0;
   }

   return 0;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int system_get_cccv(system_t *sys)
 *
 *  @brief	If FGIC is connected, query FGIC for CC and CV 
 *
 *  @return 	0 if success; negative otherwise
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int system_get_cccv(system_t *sys)
{
   int rc = 0;

   if (sys->fgic != NULL)
      rc = fgic_get_cccv(sys->fgic, &sys->I_chg, &sys->V_chg);

   return rc;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void system_destroy(system_t *sys)
 *
 *  @brief	Cleanup system
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void system_destroy(system_t *sys)
{
   if (sys != NULL) free(sys);
}


