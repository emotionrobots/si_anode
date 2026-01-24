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

#include "util.h"
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

   for (int i=0; i< sim->params_sz; i++)
   {
      char *name = sim->params[i].name;
      char *type = sim->params[i].type;
      void *value = sim->params[i].value;

      if (0==strcmp(type, "%b"))
         printf("%s (%s):  %d\n", name, type, *(bool *)value);
      else if (0==strcmp(type, "%d"))
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

   for (int i=0; i< sim->params_sz; i++)
   {
      void *value = sim->params[i].value;
      char *type = sim->params[i].type;

      if (0==strcmp(name, sim->params[i].name))
      {
         if (0==strcmp(type, "%b"))
            printf("%s (%s):  %d\n", name, type, *(bool *)value);
	 else if (0==strcmp(type, "%d"))
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
	  if (0==strcmp(sim->params[i].type, "%b"))
             *((bool *)sim->params[i].value) = atoi(value) ? true : false; 
	  else if (0==strcmp(sim->params[i].type, "%d"))
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
 *  @fn         int setup_cond(sim_t *sim, bool *res, char *lop, char *param, char *compare, double value)
 *
 *  @brief      Check condition
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
int setup_cond(sim_t *sim, int k, char* lop, char *param, char *compare, double value)
{
   char *ptype = util_get_params_type(sim, param);
   if (ptype == NULL || (0 != strcmp(ptype, "%lf"))) return -1;

   strcpy(sim->cond[k].param, param);
   sim->cond[k].compare = util_strtolop(compare);
   sim->cond[k].value = value;
   sim->cond[k].lop = util_strtolop(lop);

   return 0;
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
   (void)p_usr;

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
 *  @fn		int f_run_to(struct _menu *m, int argc, char **argv, void *p_usr)
 *
 *  @brief	Run command handler
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
int f_run_to(struct _menu *m, int argc, char **argv, void *p_usr)
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
      rc = setup_cond(sim, 0, NULL, "t", ">=", t_end);
      if (rc != 0)
      {
         printf("f_run_to: setup_cond returned rc=%d\n", rc);
         goto _err_ret;
      }
      rc = sim_run_start(sim);
   }

_err_ret:
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
      if (k==0)
      {
         trace[k].color.r = 255; 
         trace[k].color.g = 0;
         trace[k].color.b = 0;
      }
      else if (k==1)
      {
         trace[k].color.r = 0; 
         trace[k].color.g = 255;
         trace[k].color.b = 0;
      }
      else if (k==2)
      {
         trace[k].color.r = 0; 
         trace[k].color.g = 0;
         trace[k].color.b = 255;
      }
      else if (k==3)
      {
         trace[k].color.r = 128; 
         trace[k].color.g = 255;
         trace[k].color.b = 128;
      }
      else if (k==4)
      {
         trace[k].color.r = 80; 
         trace[k].color.g = 128;
         trace[k].color.b = 200;
      }
      else if (k==5)
      {
         trace[k].color.r = 180; 
         trace[k].color.g = 20;
         trace[k].color.b = 100;
      }
      else 
      {
         trace[k].color.r = 128; 
         trace[k].color.g = 128;
         trace[k].color.b = 128;
      }
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
 *
 *  @fn		int f_compare(struct _menu *m, int argc, char **argv, void *p_usr)
 *
 *  @brief	Compare fgic ecm model with batt ecm model	
 *
 *  @note	compare	
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
int f_compare(struct _menu *m, int argc, char **argv, void *p_usr)
{
   if (m==NULL || p_usr==NULL || argv==NULL) return -1;
   sim_t *sim = (sim_t *)p_usr;
   fgic_t *fgic = sim->fgic;
   batt_t *batt = sim->batt;


   if (argc != 2) 
   {
      printf("%s:\t%s (usage:%s)\n", m->name, m->desc, m->help);
      goto _err_ret;
   }
   else if (0==strcmp(argv[1], "R0"))
   {
      for (int k=0; k<SOC_GRIDS; k++)
      {
         printf("Batt_R0[%d]=%lf, FGIC_R0[%d]=%lf\n", 
                 k, batt->ecm->params->r0_tbl[k],
		 k, fgic->ecm->params->r0_tbl[k]);
      }
   }
   else if (0==strcmp(argv[1], "R1"))
   {
      for (int k=0; k<SOC_GRIDS; k++)
      {
         printf("Batt_R1[%d]=%lf, FGIC_R1[%d]=%lf\n", 
                 k, batt->ecm->params->r1_tbl[k],
		 k, fgic->ecm->params->r1_tbl[k]);
      }
   }
   else if (0==strcmp(argv[1], "C1"))
   {
      for (int k=0; k<SOC_GRIDS; k++)
      {
         printf("Batt_C1[%d]=%lf, FGIC_C1[%d]=%lf\n", 
                 k, batt->ecm->params->c1_tbl[k],
		 k, fgic->ecm->params->c1_tbl[k]);
      }
   }
   else if (0==strcmp(argv[1], "h_dsg"))
   {
      for (int k=0; k<SOC_GRIDS; k++)
      {
         printf("Batt_h_dsg[%d]=%lf, FGIC_h_dsg[%d]=%lf\n", 
                 k, batt->ecm->params->h_dsg_tbl[k],
		 k, fgic->ecm->params->h_dsg_tbl[k]);
      }
   }
   else if (0==strcmp(argv[1], "h_chg"))
   {
      for (int k=0; k<SOC_GRIDS; k++)
      {
         printf("Batt_h_chg[%d]=%lf, FGIC_h_chg[%d]=%lf\n", 
                 k, batt->ecm->params->h_chg_tbl[k],
		 k, fgic->ecm->params->h_chg_tbl[k]);
      }
   }
   else if (0==strcmp(argv[1], "V_oc"))
   {
      for (int k=0; k<SOC_GRIDS; k++)
      {
         printf("Batt_V_oc[%d]=%lf, FGIC_V_oc[%d]=%lf\n",
                 k, batt->ecm->params->ocv_tbl[k],
                 k, fgic->ecm->params->ocv_tbl[k]);
      }
   }
   else
   {
      printf("%s:\t%s (usage:%s)\n", m->name, m->desc, m->help);
      goto _err_ret;
   }

   return 0;

_err_ret:
   return -1;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int f_run_until(struct _menu *m, int argc, char **argv, void *p_usr)
 *
 *  @brief	Run until t, V, T or soc condition is reached
 *
 *  @note	run until <t | v | T | soc> <op> <value> 
 *
 *  The function sets up the conditionals then unpause the sim_loop(). sim_check_pause() will check the conditionals
 *  for pausing.
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
int f_run_until(struct _menu *m, int argc, char **argv, void *p_usr)
{
   char *endptr;

   if (m==NULL || p_usr==NULL || argv==NULL) return -1;
   sim_t *sim = (sim_t *)p_usr;

   int xargc = argc-1;
   if (xargc < 3) return -2;

   int i=0;
   int remain = xargc;

   if (!util_is_numeric(argv[i+3])) return -3;

   LOCK(&sim->mtx);
   int k = 0;
   char *lop = NULL;
   char *param = argv[i+1];
   char *comparator = argv[i+2];
   double value = strtod(argv[i+3], &endptr);
   int rc = setup_cond(sim, k++, lop, param, comparator, value);
   remain -= 3;
   if (rc != 0) return -4;

   while (remain >= 4) 
   {
      lop = argv[i+4];
      i+=4;
      if (!util_is_numeric(argv[i+3])) return -3;
      param = argv[i+1];
      comparator = argv[i+2];
      value = strtod(argv[i+3], &endptr);
      rc = setup_cond(sim, k++, lop, param, comparator, value);
      if (rc != 0) break;
      remain -= 4; 
   }
   UNLOCK(&sim->mtx);
   if (rc != 0) return -4;

   if (remain != 0) return -5; 

#if 0
   for (int j=0; j<MAX_COND; j++)
   {
      printf("cond#%d: lop=%s, param=%s, comparator=%s, value=%lf\n", 
              j, util_loptostr(sim->cond[j].lop), sim->cond[j].param, 
	      util_loptostr(sim->cond[j].compare), sim->cond[j].value);
   }
#endif

   rc = sim_run_start(sim);
   return 0;
}



/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int f_run_another(struct _menu *m, int argc, char **argv, void *p_usr)
 *
 *  @brief	Run another <t> seconds
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
int f_run_another(struct _menu *m, int argc, char **argv, void *p_usr)
{
   int rc = -1;
   char *endptr;

   if (m==NULL || p_usr==NULL || argv==NULL) return -1;
   sim_t *sim = (sim_t *)p_usr;

   if (argc != 2) return -2;

   errno = 0;
   double t_more = strtod(argv[1], &endptr);
   if (argv[1]!=endptr && errno==0)
   {
      rc = setup_cond(sim, 0, NULL, "t", ">=", sim->t + t_more);
      if (rc != 0) 
      {
         printf("f_run_another: setup_cond returned rc=%d\n", rc);
         goto _err_ret;
      }
      rc = sim_run_start(sim);
   }

_err_ret:
   return rc;
}




/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn         int f_run_script(struct _menu *m, int argc, char **argv, void *p_usr)
 *
 *  @brief      Run script seconds
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
int f_run_script(struct _menu *m, int argc, char **argv, void *p_usr)
{
   if (m==NULL || p_usr==NULL || argv==NULL) return -1;

   sim_t *sim = (sim_t *)p_usr;

   if (argc != 2) return -2;

   memset(sim->script_fn, 0, FN_LEN);
   strcpy(sim->script_fn, argv[1]);

   FILE *fp = fopen(sim->script_fn, "r");
   if (fp == NULL) return -3;

   /* script line processing loop */
   bool done = false;
   char linebuf[MAX_LINE_SZ];
   int xargc = 0;
   char *xargv[MAX_TOKENS];
   const char delim[] = " \n";
   int line_number = 0; 

   while (!done) 
   {
      line_number++;
      if (fgets(linebuf, sizeof(linebuf), fp) != NULL)
      {
         printf("-> %s\n", linebuf); 
         xargc = 0;
         memset(xargv, 0, MAX_TOKENS*sizeof(char *));
         char *token = strtok(linebuf, delim);

         while (token != NULL)
         {
            xargv[xargc] = token;
            xargc++;
            token = strtok(NULL, delim);
         }

         menu_process(sim->m_root, xargc, xargv, (void *)sim);

	 /* 
	  * Make sure the previous command is completed
	  * by checking sim->pause 
	  */
	 bool pause = false;
	 while (!pause)
         {
            LOCK(&sim->mtx);
            pause = sim->pause;
            UNLOCK(&sim->mtx);
            sched_yield();
         }
      }
      else
      {
	 done = true; 
      }
   }

   fclose(fp);
   return 0;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
int f_here(struct _menu *m, int argc, char **argv, void *p_usr)
{
   (void)m;
   (void)argc;
   (void)argv;
   (void)p_usr;

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
   (void)m;
   (void)argc;
   (void)argv;
   (void)p_usr;

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


   /* run commands */
   menu_t *m_run = menu_create("run", "run <script | to | until | another>", "", "", NULL);
   menu_add_peer(m_root, m_run);

   menu_t *m_run_to = menu_create("to", "run to <t>", "run to <t>", "", f_run_to);
   menu_add_child(m_run, m_run_to);

   menu_t *m_run_until = menu_create("until", "run until <t|V|soc> <val>", "run until <t|V|soc> <val>", "", f_run_until);
   menu_add_peer(m_run_to, m_run_until);

   menu_t *m_run_another = menu_create("another", "run another <t> seconds", "run another <t>", "", f_run_another);
   menu_add_peer(m_run_until, m_run_another);

   menu_t *m_run_script = menu_create("script", "run script <file>", "run script <file>", "", f_run_script);
   menu_add_peer(m_run_another, m_run_script);

   /* set/show commands */
   menu_t *m_set = menu_create("set", "set param value", "set <param> <value>", "", f_set);
   menu_add_peer(m_root, m_set);

   menu_t *m_show = menu_create("show", "show param value", "show | show <param>", "", f_show);
   menu_add_peer(m_root, m_show);


   /* log command */
   menu_t *m_log = menu_create("log", "log data to file", "log <start <file> <data0> <data1> ...> | <stop>", "", f_log);
   menu_add_peer(m_root, m_log);

   /* plot command */
   menu_t *m_plot = menu_create("plot", "plot a csv file", "plot <file>", "", f_plot);
   menu_add_peer(m_root, m_plot);

   /* Compare command */
   menu_t *m_compare = menu_create("compare", "compare fgic & batt ecm model", "compare", "", f_compare);
   menu_add_peer(m_root, m_compare);

   /* dummy placeholder commands */
   menu_t *m_cd = menu_create("cd", "cd command", "cd <here | there>", "", NULL);
   menu_add_peer(m_root, m_cd);

   menu_t *m_here = menu_create("here", "here commands", "here-", "", f_here);
   menu_add_child(m_cd, m_here);

   menu_t *m_there = menu_create("there", "there commands", "there-", "", f_there);
   menu_add_peer(m_here, m_there);

   
   return m_root;
}

