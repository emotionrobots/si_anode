/*!
 *======================================================================================================================
 * 
 * @file		app_menu.c
 *
 * @brief	 	App menu implementation 
 *
 *======================================================================================================================
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>

#include "menu.h"
#include "app_menu.h"
#include "scope_plot.h"



/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int show_all_params(sim_t *sim)
 *
 *  @brief	Show all parameter helper functions
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
int show_all_params(sim_t *sim)
{
   int rc = -1;
   char *endptr;


   for (int i=0; i< sim->params_sz; i++)
   {
      char *name = sim->params[i].name;
      char *type = sim->params[i].type;
      void *value = sim->params[i].value;

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



/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int show_params(sim_t *sim, char *name)
 *
 *  @brief	Show command helper function
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
int show_params(sim_t *sim, char *name)
{
   int rc = -1;
   char *endptr;

   for (int i=0; i< sim->params_sz; i++)
   {
      void *value = sim->params[i].value;
      char *type = sim->params[i].type;

      if (0==strcmp(name, sim->params[i].name))
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


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int set_params(sim_t *sim, char *name, char *value)
 *
 *  @brief	Set parameter helper function
 * 
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
int set_params(sim_t *sim, char *name, char *value)
{
   int rc = -1;
   char *endptr;

   LOCK(sim);
   for (int i=0; i < sim->params_sz; i++)
   {
       if (0==strcmp(sim->params[i].name, name))
       {

          if (0!=strcmp(sim->params[i].type, "%s") && !util_is_numeric(value))
             break;

	  if (0==strcmp(sim->params[i].type, "%d"))
             *((int *)sim->params[i].value) = atoi(value);
	  else if (0==strcmp(sim->params[i].type, "%l"))
             *((long *)sim->params[i].value) = strtol(value, &endptr, 0);
	  else if (0==strcmp(sim->params[i].type, "%f"))
             *((float *)sim->params[i].value) = strtof(value, &endptr);
	  else if (0==strcmp(sim->params[i].type, "%lf"))
             *((double *)sim->params[i].value) = strtod(value, &endptr);
          else
             strcpy((char *)sim->params[i].value, value);
	  return 0;
       }
   }
   UNLOCK(sim);

   return -1;
}

/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int f_ls(struct _menu *m, int argc, char **argv, void *p_usr)
 *
 *  @brief	ls command handler
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
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

/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int f_run(struct _menu *m, int argc, char **argv, void *p_usr)
 *
 *  @brief	Run command handler
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
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
      LOCK(&sim->mtx);   
      sim->t_end = t_end;
      sim->pause = false;
      UNLOCK(&sim->mtx);   

      rc = sim_start(sim);
   }
   return rc;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int f_set(struct _menu *m, int argc, char **argv, void *p_usr)
 *
 *  @brief	Set command handler - parameter to set defined in argv[1]
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
int f_set(struct _menu *m, int argc, char **argv, void *p_usr)
{
   int rc = -1;

   if (m==NULL || p_usr==NULL || argv==NULL) return -1;
   sim_t *sim = (sim_t *)p_usr;

   if (argc != 3) return -2;

   char *param = argv[1];
   char *value = argv[2];
   rc = set_params(sim, param, value);

   return rc;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int f_show(struct _menu *m, int argc, char **argv, void *p_usr)
 *
 *  @brief	Show parameter in argv
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
int f_show(struct _menu *m, int argc, char **argv, void *p_usr)
{
   int rc = -1;

   if (m==NULL || p_usr==NULL || argv==NULL) return -1;
   sim_t *sim = (sim_t *)p_usr;

   if (argc == 1)
      rc = show_all_params(sim);
   else if (argc == 2)
      rc = show_params(sim, argv[1]);

   return rc;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int f_log_start(struct _menu *m, int argc, char **argv, void *p_usr)
 }* *  @brief	Start Logging data to file *
 *  @note	log start <file> <data0> <data1> ...
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
int f_log(struct _menu *m, int argc, char **argv, void *p_usr)
{
   int rc = -1;
   char *endptr;

   if (m==NULL || p_usr==NULL || argv==NULL) return -1;
   sim_t *sim = (sim_t *)p_usr;

   if (argc < 2) 
      return -2;

   // log stop
   if (argc == 2)
   {
      if (0==strcmp(argv[1], "stop"))
      {
         if (sim != NULL && sim->logfp != NULL)
            fclose(sim->logfp);
	 return 0;
      }
      return -3; 
   }

   if (0!=strcmp(argv[1], "start")) return -3;

   if (strlen(argv[2]) < LOG_FN_LEN)
   {
      strcpy(sim->logfn, argv[2]); 
      if (sim->logfp != NULL) fclose(sim->logfp);
      sim->logfp = fopen(sim->logfn, "w");
      if (sim->logfp == NULL) 
      {
         printf("error: file %s open error.\n", sim->logfn);
         return -4; 
      }
     
      /* print params names  */ 
      char *data_name = NULL;
      sim->logn = 0;
      for (int n = 3; n < argc; n++)
      {
         bool found = false;
         data_name = argv[n];
         for (int i=0; i < sim->params_sz; i++) 
	 {
            if (0==strcmp(data_name, sim->params[i].name))
	    {
               if (n==argc-1)
                  fprintf(sim->logfp, "%s", sim->params[i].name); 
	       else
                  fprintf(sim->logfp, "%s,", sim->params[i].name); 

	       sim->logn++;
	       found = true;
	    }
	 }
	 if (!found)
	 {
            printf("error: variable \'%s\' not found.\n", data_name);
	    return -5;
	 }
      }

      fprintf(sim->logfp, "\n");
      rc = 0;
   }
   else 
   {
      printf("error: filename must be < %d.\n", LOG_FN_LEN); 
   }

   return rc;
}



/*!
 *---------------------------------------------------------------------------------------------------------------------
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
int f_cd(struct _menu *m, int argc, char **argv, void *p_usr)
{
   printf("f_cd called\n");
   return 0;
}

/*!
 *---------------------------------------------------------------------------------------------------------------------
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
int f_here(struct _menu *m, int argc, char **argv, void *p_usr)
{
   printf("f_here called\n");
   return 0;
}

/*!
 *---------------------------------------------------------------------------------------------------------------------
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
int f_there(struct _menu *m, int argc, char **argv, void *p_usr)
{
   printf("f_there called\n");
   return 0;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		menu_t *app_menu_init()
 *
 *  @brief	Create application menu system
 *
 *  @return	root menu
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
menu_t *app_menu_init()
{
   menu_t *m_root = menu_create("ls", "list commands", "ls", "", f_ls);

   menu_t *m_run = menu_create("run", "run simulation for a duration", "run <secs>", "", f_run);
   menu_add_peer(m_root, m_run);

   menu_t *m_set = menu_create("set", "set param value", "set <param> <value>", "", f_set);
   menu_add_peer(m_root, m_set);

   menu_t *m_show = menu_create("show", "show param value", "show | show <param>", "", f_show);
   menu_add_peer(m_root, m_show);

   menu_t *m_log = menu_create("log", "log data to file", "log <start <file> <data0> <data1> ...> | <stop>", "", f_log);
   menu_add_peer(m_root, m_log);

   menu_t *m_cd = menu_create("cd", "cd command", "cd <here | there>", "", f_cd);
   menu_add_peer(m_root, m_cd);

   menu_t *m_here = menu_create("here", "here commands", "here-", "", f_here);
   menu_add_child(m_cd, m_here);

   menu_t *m_there = menu_create("there", "there commands", "there-", "", f_there);
   menu_add_peer(m_here, m_there);

   return m_root;
}

