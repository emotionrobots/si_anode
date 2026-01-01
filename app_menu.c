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
         printf("%s (%s):  %d\n", name, type, *(int *)value);
      else if (0==strcmp(type, "%ld"))
         printf("%s (%s):  %ld\n", name, type, *(long *)value);
      else if (0==strcmp(type, "%f"))
         printf("%s (%s):  %f\n", name, type, *(float *)value);
      else if (0==strcmp(type, "%lf"))
         printf("%s (%s):  %lf\n", name, type, *(double *)value);
      else
         printf("%s (%s):  %s\n", name, type, (char *)value);
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
            printf("%s (%s):  %d\n", name, type, *(int *)value);
         else if (0==strcmp(type, "%ld"))
            printf("%s (%s):  %ld\n", name, type, *(long *)value);
         else if (0==strcmp(type, "%f"))
            printf("%s (%s):  %f\n", name, type, *(float *)value);
         else if (0==strcmp(type, "%lf"))
            printf("%s (%s):  %lf\n", name, type, *(double *)value);
         else
            printf("%s (%s):  %s\n", name, type, (char *)value);
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

   LOCK(&sim->mtx);
   for (int i=0; i < sim->params_sz; i++)
   {
       if (0==strcmp(sim->params[i].name, name))
       {
	  if (0==strcmp(sim->params[i].type, "%d"))
             *((int *)sim->params[i].value) = atoi(value);
	  else if (0==strcmp(sim->params[i].type, "%ld"))
             *((long *)sim->params[i].value) = strtol(value, &endptr, 0);
	  else if (0==strcmp(sim->params[i].type, "%f"))
             *((float *)sim->params[i].value) = strtof(value, &endptr);
	  else if (0==strcmp(sim->params[i].type, "%lf"))
             *((double *)sim->params[i].value) = strtod(value, &endptr);
          else
             strcpy((char *)sim->params[i].value, value);
	  rc = 0;
	  break;
       }
   }
   UNLOCK(&sim->mtx);

   return rc;
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
 *  @fn		int f_log(struct _menu *m, int argc, char **argv, void *p_usr)
 *
 *  @brief	Start/stop Logging data to file
 *
 *  @note	log <start <file> <data0> <data1> ...> | <stop>
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

   if (strlen(argv[2]) < FN_LEN)
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
      fprintf(sim->logfp, "t "); 
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
                  fprintf(sim->logfp, "%s ", sim->params[i].name); 

	       sim->logi[sim->logn++] = i;
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
      printf("error: filename must be < %d.\n", FN_LEN); 
   }

   return rc;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int f_plot(struct _menu *m, int argc, char **argv, void *p_usr)
 *
 *  @brief	Plot a saved CSV file
 *
 *  @note	plot <file> 
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
int f_plot(struct _menu *m, int argc, char **argv, void *p_usr)
{
   int rc = 0;
   char *endptr;
   int idx[MAX_PARAMS];
   char linebuf[MAX_LINE_SZ];
   char titlebuf[MAX_LINE_SZ];
   int nvars = 0;
   const char delim[] = " \n";
   scope_trace_desc_t trace[MAX_PARAMS];
   scope_plot_t *plot=NULL;
   SDL_Window *win = NULL;
   SDL_Renderer *ren = NULL;


   if (m==NULL || p_usr==NULL || argv==NULL) return -1;
   sim_t *sim = (sim_t *)p_usr;

   if (argc != 2)
   {
      rc = -1;
      goto _err_ret;
   }

   /* init scope trace object */
   memset(trace, 0, sizeof(scope_trace_desc_t));
   for (int k=0; k<MAX_PARAMS; k++)
   {
      trace[k].name = "<no_name>";
      trace[k].color.r = 255; 
      trace[k].color.g = 255;
      trace[k].color.b = 255;
      trace[k].color.a = 255;
   }

   FILE *fp = NULL;
   if ((strlen(argv[1]) < FN_LEN) && (fp = fopen(argv[1], "r")) != NULL)
   {
      /* create SDL window */
      if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0)
      { 
         win = SDL_CreateWindow(
            "scope_plot demo (SDL2)",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            1100, 650,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
         );

         if (!win) 
	 {
            rc = -4;
	    goto _err_ret;
         }

         ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
         if (!ren) 
	 {
            rc = -5;
	    goto _err_ret;
         }


         /* read column titles */
         if (fgets(titlebuf, sizeof(titlebuf), fp) != NULL)
         {
            /* read  't' */
            char *token = strtok(titlebuf, delim);
            if (0!=strcmp(token, "t")) 
	    {
	       rc = -6;
	       goto _err_ret;
            }

	    /* Process rest of the title line */ 
	    nvars = 0;
	    while ( (token = strtok(NULL, delim)) != NULL)
	    {
               bool found = false;
               bool valid = false;
	       
	       /* check if trace valid */
               for (int k=0; k < sim->params_sz; k++)
	       {
		  if (0==strcmp(token, sim->params[k].name))
		  {
		     found = true;
		     if (0==strcmp("%d", sim->params[k].type) ||
		         0==strcmp("%ld", sim->params[k].type) ||  
		         0==strcmp("%f", sim->params[k].type) ||  
		         0==strcmp("%lf", sim->params[k].type))
		     {
                        trace[nvars++].name = token;
			valid = true;
		     }
		     break;
		  }
               }

	       if (!found || !valid)
	       {
                  printf("error: found =%d or invalid=%d\n", found, valid);
		  rc = -7;
		  goto _err_ret; 
	       }
	       
	    } // while ( (token = strtok(NULL, delim)) != NULL)
	      
	 } // if (fgets(titlebuf, sizeof(titlebuf), fp) != NULL)

	 /* setup config */
	 scope_plot_cfg_t cfg = scope_plot_default_cfg();
	 cfg.max_points = MAX_PLOT_PTS;
	 cfg.font_px = 14;

         plot = scope_plot_create(win, ren, nvars, trace, &cfg);
         if (plot==NULL) 
	 {
            rc = -8;
	    goto _err_ret;
         }

         scope_plot_set_title(plot, "Signal Scope");

	 SDL_Color bkg_color;
	 bkg_color.r=12;
	 bkg_color.b=12; 
	 bkg_color.g=16; 
	 bkg_color.a=255;

         scope_plot_set_background(plot, bkg_color);
 
         double x = 0;
	 double xmin=1e10, xmax=-1e10;
         double *y = (double *)calloc(nvars, sizeof(double));
	 char *endptr;
         while (fgets(linebuf, sizeof(linebuf), fp) != NULL)
	 {
            /* get x (or t) */
            char *token = strtok(linebuf, delim);
	    x = strtod(token, &endptr);
            if (x < xmin) xmin = x;
            if (x > xmax) xmax = x;

	    /* get y */
	    int i = 0;
	    bool done = false;
            while (!done)
	    {
	       token = strtok(NULL, delim); 
	       if (token != NULL)
	       {
                  if (util_is_numeric(token))
                     y[i] = strtod(token, &endptr);
                  else
                     y[i] = 0;
                  i++;
	       }
	       else
	       {
                  done = true;
	       }
	    } 

	    if (i != nvars)
	    {
               printf("error: parsing i=%d nvar=%d\n", i, nvars);
	       free(y);
	       rc= -1;
	       goto _err_ret;
            }
	    scope_plot_push(plot, x, y);

	 } // while (fgets(linebuf, sizeof(linebuf), fp) != NULL)
         free(y); 
	   
	  
	 scope_plot_set_x_range(plot, xmin, xmax); 
	 scope_plot_render(plot);
	 SDL_RenderPresent(ren);


	 /* pause graph to wait for key */
	 bool done = false;
         SDL_Event e;	
	 while (!done)
	 {
            SDL_PollEvent(&e); 
            if (e.type == SDL_QUIT) 
	    {
	       rc = 0;
	       goto _err_ret;
	    }
	    else if (e.type == SDL_KEYDOWN) 
	    {
               done = e.key.keysym.sym == SDLK_ESCAPE;
	    }
	 }


      } // if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0) 

   } // if ((strlen(argv[1]) < FN_LEN) && (fp = fopen(argv[1], "r")) != NULL)
     
_err_ret:
   if (plot != NULL) scope_plot_destroy(plot);
   if (ren != NULL) SDL_DestroyRenderer(ren);
   if (ren != NULL) SDL_DestroyWindow(win);
   SDL_Quit();

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

   menu_t *m_plot = menu_create("plot", "plot a csv file", "plot <file>", "", f_plot);
   menu_add_peer(m_root, m_plot);

   menu_t *m_cd = menu_create("cd", "cd command", "cd <here | there>", "", f_cd);
   menu_add_peer(m_root, m_cd);

   menu_t *m_here = menu_create("here", "here commands", "here-", "", f_here);
   menu_add_child(m_cd, m_here);

   menu_t *m_there = menu_create("there", "there commands", "there-", "", f_there);
   menu_add_peer(m_here, m_there);

   return m_root;
}

