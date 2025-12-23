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
#include <string.h>
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
   bool realtime = false;
   char linebuf[MAX_LINE_SZ];
   const char delim[] = " \n";

   sim_init(&sim);
   while (!done)
   {
      memset(linebuf, 0, sizeof(linebuf));
      printf("> ");
      if (fgets(linebuf, sizeof(linebuf), stdin) != NULL)
      {
         char *token = strtok(linebuf, delim);
         while (token != NULL) 
	 {
            if (0==strcmp(token, "quit"))
               goto _quit;
	    
            printf("Token: %s\n", token);
	    token = strtok(NULL, delim);
	 }
      }
      else
      {
         printf("Error reading input.\n");
      }

#if 0
      if (!realtime)
      {
         sim_update(&sim);
         printf("sim->t=%.2lf\n", sim.t);
      }
#endif
   }

_quit:
   sim_cleanup(&sim);
}

#undef __APP_C__
