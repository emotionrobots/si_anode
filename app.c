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

typedef struct
{
   char *name;	  
   char *type;
   void *value;
}
param_t;

double tt = 0.0f;
int jj = 0;
char kk[256] = {0};

param_t g_params[] = {
   { .name="tt", .type="%lf", .value=&tt },
   { .name="jj", .type="%d", .value=&jj },
   { .name="kk", .type="%s", .value=kk}
};



int show_all_params()
{
   int rc = -1;
   char *endptr;

   for (int i=0; i< sizeof(g_params)/sizeof(param_t); i++)
   {
      char *name = g_params[i].name;
      char *type = g_params[i].type;
      void *value = g_params[i].value;

      if (0==strcmp(type, "%d"))
         printf("%s:\t type:%s\t value:%d\n", name, type, *(int *)value);
      else if (0==strcmp(type, "%ld"))
         printf("%s:\t type:%s\t value:%ld\n", name, type, *(long *)value);
      else if (0==strcmp(type, "%f"))
         printf("%s:\t type:%s\t value:%f\n", name, type, *(float *)value);
      else if (0==strcmp(type, "%lf"))
         printf("%s:\t type:%s\t value:%lf\n", name, type, *(double *)value);
      else 
         printf("%s:\t type:%s\t value:%s\n", name, type, (char *)value);
      rc = 0;
   }

   return rc;
}


bool is_numeric(char *str)
{
  char *endptr;
  strtod(str, &endptr);
  return (str!=endptr);
}


int show_params(char *name)
{
   int rc = -1;
   char *endptr;

   for (int i=0; i< sizeof(g_params)/sizeof(param_t); i++)
   {
      void *value = g_params[i].value;
      char *type = g_params[i].type;

      if (0==strcmp(name, g_params[i].name))
      { 
         if (0==strcmp(type, "%d"))
            printf("%s:\t type:%s\t value:%d\n", name, type, *(int *)value);
         else if (0==strcmp(type, "%ld"))
            printf("%s:\t type:%s\t value:%ld\n", name, type, *(long *)value);
         else if (0==strcmp(type, "%f"))
            printf("%s:\t type:%s\t value:%f\n", name, type, *(float *)value);
         else if (0==strcmp(type, "%lf"))
            printf("%s:\t type:%s\t value:%lf\n", name, type, *(double *)value);
         else 
            printf("%s:\t type:%s\t value:%s\n", name, type, (char *)value);
         rc = 0;
	 break;
      }
   }

   return rc;
}


int set_params(char *name, char *value)
{
   int rc = -1;
   char *endptr;

   for (int i=0; i< sizeof(g_params)/sizeof(param_t); i++)
   {
       if (0==strcmp(g_params[i].name, name))
       {
          if (0!=strcmp(g_params[i].type, "%s") && !is_numeric(value)) 
             break; 

	  if (0==strcmp(g_params[i].type, "%d"))
             *((int *)g_params[i].value) = atoi(value);
	  else if (0==strcmp(g_params[i].type, "%l"))
             *((long *)g_params[i].value) = strtol(value, &endptr, 0);
	  else if (0==strcmp(g_params[i].type, "%f"))
             *((float *)g_params[i].value) = strtof(value, &endptr);
	  else if (0==strcmp(g_params[i].type, "%lf"))
             *((double *)g_params[i].value) = strtod(value, &endptr);
          else
             strcpy((char *)g_params[i].value, value);
	  return 0;
       }
   }

   return -1; 
}


int f_ls(struct _menu *m, int argc, char **argv, void *p_usr)
{
   (void)argc;
   (void)argv;

   while (m != NULL) 
   {
      printf("%s:\t%s (usage:%s)\n", m->name, m->desc, m->help);
      m = m->next;
   }
   printf("\n"); 
   return 0;
}


int f_run(struct _menu *m, int argc, char **argv, void *p_usr)
{
   int rc = -1;
   char *endptr;

   if (m==NULL || p_usr==NULL || argv==NULL) return -1;
   sim_t *sim = (sim_t *)p_usr;

   if (argc != 2) return -2; 

   errno = 0; 
   double t_end = strtod(argv[1], &endptr);
   if (argv[1]!=endptr && errno==0)
   {
      bool done = false;
      while (!done)
      {
         rc = sim_update(sim);
	 if (sim->t >= t_end || rc < 0) 
            done = true;
      }
   }
   return rc;
}


int f_set(struct _menu *m, int argc, char **argv, void *p_usr)
{
   int rc = -1;

   if (m==NULL || p_usr==NULL || argv==NULL) return -1;
   sim_t *sim = (sim_t *)p_usr;

   if (argc != 3) return -2; 

   char *param = argv[1];
   char *value = argv[2];
   rc = set_params(param, value);

   return rc;
}


int f_show(struct _menu *m, int argc, char **argv, void *p_usr)
{
   int rc = -1;

   if (m==NULL || p_usr==NULL || argv==NULL) return -1;
   sim_t *sim = (sim_t *)p_usr;

   if (argc == 1) 
      rc = show_all_params(); 
   else if (argc == 2)
      rc = show_params(argv[1]);

   return rc;
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


   menu_t *m_root = menu_create("ls", "list commands", "ls", "", f_ls);

   menu_t *m_run = menu_create("run", "run simulation for a duration", "run <secs>", "", f_run);
   rc = menu_add_peer(m_root, m_run);

   menu_t *m_set = menu_create("set", "set param value", "set <param> <value>", "", f_set);
   rc = menu_add_peer(m_root, m_set);

   menu_t *m_show = menu_create("show", "show param value", "show | show <param>", "", f_show);
   rc = menu_add_peer(m_root, m_show);

   menu_t *m_cd = menu_create("cd", "cd commands", "cd <here | there>", "", f_cd);
   menu_add_peer(m_root, m_cd);

   menu_t *m_here = menu_create("here", "here commands", "here-", "", f_here);
   menu_add_child(m_cd, m_here);

   menu_t *m_there = menu_create("there", "there commands", "there-", "", f_there);
   menu_add_peer(m_here, m_there);


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

         menu_process(m_root, argc, argv, (void *)&sim);
      }
      else
      {
         printf("Error reading input.\n");
      }
   }

_quit:
   sim_cleanup(&sim);
}

#undef __APP_C__
