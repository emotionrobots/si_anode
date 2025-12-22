/*!
 *======================================================================================================================
 * 
 * @file		app.c
 *
 * @brief		Run FGIC code against 'true' battery
 *
 *======================================================================================================================
 */
#define __APP_C__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>
#include <time.h>
#include <errno.h>

#include "globals.h"
#include "flash_params.h"
#include "sim.h"
#include "util.h"


/*!
 *----------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		main()
 *
 *  @brief	Main application logic
 *
 *----------------------------------------------------------------------------------------------------------------------
 */
int main()
{
   sim_t sim;
   bool done = false;


   sim_init(&sim);

   while (!done)
   {
#ifdef REALTIME
      if (util_msleep(1000L) < 0)
      {
         printf("util_sleep() error\n");
         done = true;
      }  
#else
      sim_update(&sim);
      printf("sim->t=%.2lf\n", sim.t);
#endif
   }

   sim_cleanup(&sim);
}

#undef __APP_C__
