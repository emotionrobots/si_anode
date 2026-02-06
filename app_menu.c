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
      fprintf(sim->logfp, "t,"); 
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
 *  @fn		int split_csv_line(char *line, char **out, int max_out) 
 *
 *  @brief	Read a CSV line and separate out the fields
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
int split_csv_line(char *line, char **out, int max_out)
{
    // Simple CSV splitter (no quotes) good enough for numeric logs
    int n = 0;
    char *p = line;
    while (p && *p && n < max_out)
    {
        // trim leading spaces
        while (*p == ' ' || *p == '\t') p++;
        out[n++] = p;
        char *comma = strchr(p, ',');
        if (!comma) break;
        *comma = '\0';
        p = comma + 1;
    }

    // trim trailing newline/spaces for each token
    for (int i = 0; i < n; i++)
    {
        char *s = out[i];
        size_t L = strlen(s);
        while (L && (s[L-1] == '\n' || s[L-1] == '\r' || s[L-1] == ' ' || s[L-1] == '\t')) {
            s[L-1] = '\0';
            L--;
        }
    }
    return n;
}

/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		SDL_Color palette(int i)
 *
 *  @brief	Returns the color palette of a trace
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
SDL_Color palette(int i)
{
    static SDL_Color p[] = {
        {255,  80,  80, 255},
        { 80, 255,  80, 255},
        { 80, 160, 255, 255},
        {255, 200,  80, 255},
        {220,  80, 255, 255},
        { 80, 255, 220, 255},
    };
    return p[i % (int)(sizeof(p)/sizeof(p[0]))];
}


/*!
 *--------------------------------------------------------------------------------------------------------------------- 
 *
 *  @fn		bool trace_name_valid(sim_t *sim, char *trace_name)
 *
 *  @brief	Check if trace name is valid
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
bool trace_name_valid(sim_t *sim, char *trace_name)
{
   bool valid = false;

   /* check if trace valid */
   for (int k=0; k < sim->params_sz; k++)
   {
      if (0==strcmp(trace_name, sim->params[k].name))
      {
         if (0==strcmp("%d", sim->params[k].type)  ||
             0==strcmp("%ld", sim->params[k].type) ||
             0==strcmp("%f", sim->params[k].type)  ||
             0==strcmp("%lf", sim->params[k].type))
         {
            valid = true;
         }
         break;
      }
   }

   return valid;
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
   scope_plot_t *p = NULL;
   SDL_Window *win = NULL;
   SDL_Renderer *ren = NULL;


   // Get sim pointer 
   if (m==NULL || p_usr==NULL || argv==NULL) return -1;
   sim_t *sim = (sim_t *)p_usr;
   if (argc != 2) { rc = -1; goto _err_ret; }

   /* Setup data labels */
   const char *csv_path = argv[1];
   FILE *f = fopen(csv_path, "r");
   if (!f) { rc = -2; goto _err_ret; }

   // Read first line and parse data labels
   char line[4096];
   if (!fgets(line, sizeof(line), f)) { rc = -3; goto _err_ret; }

   char *cols[64] = {0};
   int ncol = split_csv_line(line, cols, 64);
   if (ncol < 2) { rc = -4; goto _err_ret; }

   // Setup X labels
   const char *x_label = cols[0];
   int trace_count = ncol - 1;

   // Init scope trace object 
   scope_trace_desc_t tr[MAX_PARAMS];
   memset(tr, 0, sizeof(scope_trace_desc_t));
   for (int i = 0; i < trace_count; i++) 
   {
      /* check if trace valid */
      if (trace_name_valid(sim, cols[i+1]))
      {
         tr[i].name = str_dup(cols[i+1]); 
         tr[i].color = palette(i);
      }
   }


   // Init SDL renderer
   if (SDL_Init(SDL_INIT_VIDEO) != 0) { rc = -5; goto _err_ret; }
   win = SDL_CreateWindow("ScopeTrace", 
		          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                          1100, 650, SDL_WINDOW_SHOWN);
   if (!win) { rc = -6; goto _err_ret; }

   ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
   if (!ren) { rc = -7; goto _err_ret; }


   // Create plot object with default config
   scope_plot_cfg_t cfg = scope_plot_default_cfg();
   p = scope_plot_create(win, ren, trace_count, tr, &cfg);
   if (!p) { rc = -8; goto _err_ret; }

   // Set plot title and X label
   scope_plot_set_title(p, "ScopeTrace");
   scope_plot_set_x_label(p, x_label);

   // Read data into plot one line at a time.  Data buffer overflows at 40000 data ponts
   double x_min = 0.0, x_max = 1.0;
   bool first = true;
   double *y = (double*)calloc((size_t)trace_count, sizeof(double));
   if (!y) { rc = -9; goto _err_ret; }
   while (fgets(line, sizeof(line), f)) 
   {
      if (line[0] == '\0' || line[0] == '\n' || line[0] == '\r') continue;

      char *tok[64] = {0};
      int nt = split_csv_line(line, tok, 64);
      if (nt != ncol) continue; // skip malformed rows

      double x = strtod(tok[0], NULL);
      for (int i = 0; i < trace_count; i++) y[i] = strtod(tok[i+1], NULL);

      if (first) { x_min = x_max = x; first = false; }
      else { if (x < x_min) x_min = x; if (x > x_max) x_max = x; }

      scope_plot_push(p, x, y);
   }
   fclose(f);

   // Render plot
   scope_plot_set_x_range(p, x_min, x_max);
   scope_plot_render(p);
   SDL_RenderPresent(ren);

   // Event loop
   bool quit = false;
   while (!quit) 
   {
      SDL_Event e;
      while (SDL_PollEvent(&e)) 
      {
         if (e.type == SDL_QUIT) quit = true;
         if (e.type == SDL_KEYDOWN) {
             SDL_Keycode k = e.key.keysym.sym;
             if (k == SDLK_ESCAPE || k == SDLK_q) quit = true;
         }
      }
      SDL_Delay(16);
   }


_err_ret:
   if (p != NULL) scope_plot_destroy(p);
   if (ren != NULL) SDL_DestroyRenderer(ren);
   if (win != NULL) SDL_DestroyWindow(win);
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
         double r0_batt = batt->ecm->params.r0_tbl[k];
         double r0_fgic = fgic->ecm->params.r0_tbl[k];
         double ratio = fabs((r0_batt-r0_fgic)/r0_batt);
         printf("Batt_R0[%d]=%lf, FGIC_R0[%d]=%lf, error=%lf\n", k, r0_batt, k, r0_fgic, ratio) ;
      }
   }
   else if (0==strcmp(argv[1], "R1"))
   {
      for (int k=0; k<SOC_GRIDS; k++)
      {
         double r1_batt = batt->ecm->params.r1_tbl[k];
         double r1_fgic = fgic->ecm->params.r1_tbl[k];
         double ratio = fabs((r1_batt-r1_fgic)/r1_batt);
         printf("Batt_R1[%d]=%lf, FGIC_R1[%d]=%lf, error=%lf\n", k, r1_batt, k, r1_fgic, ratio) ;
      }
   }
   else if (0==strcmp(argv[1], "C1"))
   {
      for (int k=0; k<SOC_GRIDS; k++)
      {
         double c1_batt = batt->ecm->params.c1_tbl[k];
         double c1_fgic = fgic->ecm->params.c1_tbl[k];
         double ratio = fabs((c1_batt-c1_fgic)/c1_batt);
         printf("Batt_R1[%d]=%lf, FGIC_R1[%d]=%lf, error=%lf\n", k, c1_batt, k, c1_fgic, ratio) ;
      }
   }
   else if (0==strcmp(argv[1], "h_dsg"))
   {
      for (int k=0; k<SOC_GRIDS; k++)
      {
         double h_batt = batt->ecm->params.h_dsg_tbl[k];
         double h_fgic = fgic->ecm->params.h_dsg_tbl[k];
         double ratio = fabs((h_batt-h_fgic)/h_batt);
         printf("Batt_H_dsg[%d]=%lf, FGIC_H_dsg[%d]=%lf, error=%lf\n", k, h_batt, k, h_fgic, ratio) ;
      }
   }
   else if (0==strcmp(argv[1], "h_chg"))
   {
      for (int k=0; k<SOC_GRIDS; k++)
      {
         double h_batt = batt->ecm->params.h_chg_tbl[k];
         double h_fgic = fgic->ecm->params.h_chg_tbl[k];
         double ratio = fabs((h_batt-h_fgic)/h_batt);
         printf("Batt_H_chg[%d]=%lf, FGIC_H_chg[%d]=%lf, error=%lf\n", k, h_batt, k, h_fgic, ratio) ;
      }
   }
   else if (0==strcmp(argv[1], "V_oc"))
   {
      for (int k=0; k<SOC_GRIDS; k++)
      {
         double ocv_batt = batt->ecm->params.ocv_tbl[k];
         double ocv_fgic = fgic->ecm->params.ocv_tbl[k];
         double ratio = fabs((ocv_batt-ocv_fgic)/ocv_batt);
         printf("Batt_V_oc[%d]=%lf, FGIC_V_oc[%d]=%lf, error=%lf\n", k, ocv_batt, k, ocv_fgic, ratio) ;
      }
   }
   else if (0==strcmp(argv[1], "Tau"))
   {
      for (int k=0; k<SOC_GRIDS; k++)
      {
         double tau_batt = batt->ecm->params.r1_tbl[k] * batt->ecm->params.c1_tbl[k];
         double tau_fgic = fgic->ecm->params.r1_tbl[k] * fgic->ecm->params.c1_tbl[k];
         double ratio = fabs((tau_batt-tau_fgic)/tau_batt);
         printf("Batt_Tau[%d]=%lf, FGIC_Tau[%d]=%lf, error=%lf\n", k, tau_batt, k, tau_fgic, ratio) ;
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

#if 0
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
#endif


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

#if 0
   /* dummy placeholder commands */
   menu_t *m_cd = menu_create("cd", "cd command", "cd <here | there>", "", NULL);
   menu_add_peer(m_root, m_cd);

   menu_t *m_here = menu_create("here", "here commands", "here-", "", f_here);
   menu_add_child(m_cd, m_here);

   menu_t *m_there = menu_create("there", "there commands", "there-", "", f_there);
   menu_add_peer(m_here, m_there);
#endif
   
   return m_root;
}

