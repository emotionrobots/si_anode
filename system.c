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
 *  @fn		int system_init(system_t *sys)
 *
 *  @brief	System init
 *
 *  @return 	0 if success; negative otherwise
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int system_init(system_t *sys)
{
   sys->I_load = 0.0;
   sys->V_chg = DEFAULT_CV;
   sys->I_chg = DEFAULT_CC;
   sys->fgic = NULL;

   return 0;
}

/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int system_connect_fgic(system_t *sys, fgic_t *fgic)
 *
 *  @brief	Connect system to FGIC
 *
 *  @return 	0 if success; negative otherwise
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int system_connect_fgic(system_t *sys, fgic_t *fgic)
{
   if (sys == NULL || fgic == NULL) return -1;

   sys->fgic = fgic;

   return 0;
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
   if (sys == NULL) return -1;
   
   if (t > 0 && t < MAX_RUN_TIME)
   {
      if (t >= 100.0 && t < 200.0)  
         sys->I_load = 0.0;
      else if (t >= 200.0 && t < 300.0)
         sys->I_load = 1.0;
      else if (t >= 300.0 && t < 500.0)
         sys->I_load = 0.0;
      else if (t >= 500.0 && t < 600.0)
         sys->I_load = -1.0;
      else
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
 *  @fn		void system_cleanup(system_t *sys)
 *
 *  @brief	Cleanup system
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void system_cleanup(system_t *sys)
{
}


