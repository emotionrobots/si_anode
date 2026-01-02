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
double _pulsed_load(double now, double t_start, double per, double dutycycle, double I_on, double I_off)
{
   double t = now-t_start; 
   double rem = t - floor(t/per) * per;
   double ratio = rem/per;
   return (ratio < dutycycle) ? I_on : I_off;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn         double _osc_load(double now, double t_start, double per, double I_hi, double I_lo)
 *
 *  @brief      Create an sinusoidal load (oscillating)
 *
 *  @param      t:              present time
 *  @param      t_start:        pulse train origin time
 *  @param      per:            pulse period
 *  @param      I_hi:           high current
 *  @param      I_lo:           low current
 *
 *  @return     I_load
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
double _osc_load(double now, double t_start, double per, double I_on, double I_off)
{
   double t = now-t_start;
   double amp = I_on - I_off;
   double offset = (I_on + I_off)/2.0;
   return amp * sin(2.0*M_PI*t/per) + offset;
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
   if (sys==NULL || fgic==NULL) return NULL;

   sys->fgic = fgic;
   sys->load_type = DEFAULT_SYS_LOAD;
   sys->V_chg = fgic->V_chg;
   sys->I_chg = fgic->I_chg;

   sys->I = 0.0;
   sys->V = 0.0;

   sys->per = 100.0;
   sys->dutycycle = 0.5;
   sys->I_on = fabs(sys->I_chg);
   sys->I_off = 0;
   sys->t_start = 0;

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
      if (sys->load_type==SYS_LOAD_PULSE)
         sys->I = _pulsed_load(t, sys->t_start, sys->per, sys->dutycycle, sys->I_on, sys->I_off);
      else if (sys->load_type==SYS_LOAD_OSC)
         sys->I = _osc_load(t, sys->t_start, sys->per, sys->I_on, sys->I_off);
   }
   else
   {
      sys->I = 0.0;
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


