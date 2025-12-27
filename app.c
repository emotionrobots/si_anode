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
#include "menu.h"
#include "app_menu.h"



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
   int rc = 0;
   bool done = false;
   bool realtime = false;
   char linebuf[MAX_LINE_SZ];
   int argc = 0;
   char *argv[MAX_TOKENS];
   const char delim[] = " \r\n";


   sim_t *sim = sim_create(0.0, DT, TEMP_0);
   menu_t *m_root = app_menu_init(sim);

   while (!done)
   {
      printf("> ");
      if (fgets(linebuf, sizeof(linebuf), stdin) != NULL)
      {
         argc = 0;
         memset(argv, 0, MAX_TOKENS*sizeof(char *));
         char *token = strtok(linebuf, delim);

	 if (token != NULL && 0==strcmp(token, "quit"))
            goto _quit;

	 while (token != NULL) 
	 {
	    argv[argc] = token;
	    argc++;
            token = strtok(NULL, delim);
	 }

         menu_process(m_root, argc, argv, (void *)sim);
      }
      else
      {
         printf("Error reading input.\n");
      }
   }

_quit:
   sim_destroy(sim);
}

#undef __APP_C__
