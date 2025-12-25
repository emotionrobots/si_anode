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


int f_ls(struct _menu *m, int argc, char **argv, void *p_usr)
{
   (void)argc;
   (void)argv;

   while (m != NULL) 
   {
      printf("%s  ", m->name);
      m = m->next;
   }
   printf("\n"); 
   return 0;
}


int f_run(struct _menu *m, int argc, char **argv, void *p_usr)
{
   printf("f_run called\n");
   return 0;
}


int f_cd(struct _menu *m, int argc, char **argv, void *p_usr)
{
   printf("f_cd called\n");
   return 0;
}

int f_here(struct _menu *m, int argc, char **argv, void *p_usr)
{
   printf("f_here called\n");
   return 0;
}

int f_there(struct _menu *m, int argc, char **argv, void *p_usr)
{
   printf("f_there called\n");
   return 0;
}


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
   sim_t sim;
   bool done = false;
   bool realtime = false;
   char linebuf[MAX_LINE_SZ];
   int argc = 0;
   char *argv[MAX_TOKENS];
   const char delim[] = " \r\n";


   menu_t *root = menu_create("ls", "list commands", "", "", f_ls);

   menu_t *run = menu_create("run", "run commands", "", "", f_run);
   rc = menu_add_peer(root, run);

   menu_t *cd = menu_create("cd", "cd commands", "cd <here | there>", "", f_cd);
   menu_add_peer(root, cd);

   menu_t *here = menu_create("here", "here commands", "", "", f_here);
   menu_add_child(cd, here);

   menu_t *there = menu_create("there", "there commands", "", "", f_there);
   menu_add_peer(here, there);


   sim_init(&sim);

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

         menu_process(root, argc, argv);
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
