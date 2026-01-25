/*!
 *=====================================================================================================================
 *
 *  @file		sim.c
 *
 *  @brief		Simulation object implementation 
 *
 *=====================================================================================================================
 */
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <inttypes.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>

#include "globals.h"
#include "flash_params.h"
#include "util.h"
#include "sim.h"
#include "scope_plot.h"


extern flash_params_t g_batt_flash_params;
extern flash_params_t g_fgic_flash_params;


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int params_init(sim_t *sim)
 *
 *  @brief	string-enabled parameter init
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
int params_init(sim_t *sim)
{
   int i=0;

   if (sim == NULL) return -1;

   batt_t *batt = sim->batt;
   fgic_t *fgic = sim->fgic;
   system_t *system = sim->system;

   if (batt==NULL || fgic==NULL || system==NULL) return -1;

   sim->params[i].name = "realtime";
   sim->params[i].type = "%b";
   sim->params[i++].value= &sim->realtime;

   sim->params[i].name = "t";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->t;

   sim->params[i].name = "dt";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->dt;

   sim->params[i].name = "T_amb_C";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->T_amb_C;

   sim->params[i].name = "soc_batt";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->batt->ecm->soc;

   sim->params[i].name = "V_batt";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->batt->ecm->V_batt;

   sim->params[i].name = "I_batt";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->batt->ecm->I;

   sim->params[i].name = "T_batt";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->batt->ecm->T_C;

   sim->params[i].name = "H_batt";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->batt->ecm->H;

   sim->params[i].name = "R0_batt";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->batt->ecm->R0;

   sim->params[i].name = "R1_batt";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->batt->ecm->R1;

   sim->params[i].name = "C1_batt";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->batt->ecm->C1;

   sim->params[i].name = "Qmax_batt";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->batt->ecm->Q_Ah;

   sim->params[i].name = "Cp_batt";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->batt->ecm->Cp;

   sim->params[i].name = "ht_batt";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->batt->ecm->ht;

   sim->params[i].name = "Ea_R0_batt";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->batt->ecm->Ea_R0;

   sim->params[i].name = "Ea_R1_batt";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->batt->ecm->Ea_R1;

   sim->params[i].name = "Ea_C1_batt";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->batt->ecm->Ea_C1;

   sim->params[i].name = "chg_state_batt";
   sim->params[i].type = "%d";
   sim->params[i++].value= &sim->batt->ecm->chg_state;

   sim->params[i].name = "prev_chg_state_batt";
   sim->params[i].type = "%d";
   sim->params[i++].value= &sim->batt->ecm->prev_chg_state;

   sim->params[i].name = "I_quit_batt";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->batt->ecm->I_quit;

   sim->params[i].name = "V_oc_batt";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->batt->ecm->V_oc;

   sim->params[i].name = "V_rc_batt";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->batt->ecm->V_rc;

   sim->params[i].name = "I_sys";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->system->I;

   sim->params[i].name = "V_sys";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->system->V;

   sim->params[i].name = "V_chg_sys";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->system->V_chg;

   sim->params[i].name = "I_chg_sys";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->system->I_chg;

   sim->params[i].name = "load_type";
   sim->params[i].type = "%d";
   sim->params[i++].value= &sim->system->load_type;

   sim->params[i].name = "I_on";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->system->I_on;

   sim->params[i].name = "I_off";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->system->I_off;

   sim->params[i].name = "period";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->system->per;

   sim->params[i].name = "dutycycle";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->system->dutycycle;

   sim->params[i].name = "t_start_sys";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->system->t_start;

   sim->params[i].name = "V_meas_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->V_meas;

   sim->params[i].name = "I_meas_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->I_meas;

   sim->params[i].name = "T_meas_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->T_meas;

   sim->params[i].name = "soc_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->ecm->soc;

   sim->params[i].name = "V_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->ecm->V_batt;

   sim->params[i].name = "V_prev_batt_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->ecm->prev_V_batt;

   sim->params[i].name = "I_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->ecm->I;

   sim->params[i].name = "I_prev_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->ecm->prev_I;

   sim->params[i].name = "T_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->ecm->T_C;

   sim->params[i].name = "H_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->ecm->H;

   sim->params[i].name = "R0_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->ecm->R0;

   sim->params[i].name = "R1_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->ecm->R1;

   sim->params[i].name = "C1_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->ecm->C1;

   sim->params[i].name = "Qmax_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->ecm->Q_Ah;

   sim->params[i].name = "Cp_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->ecm->Cp;

   sim->params[i].name = "ht_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->ecm->ht;

   sim->params[i].name = "Ea_R0_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->ecm->Ea_R0;

   sim->params[i].name = "Ea_R1_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->ecm->Ea_R1;

   sim->params[i].name = "Ea_C1_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->ecm->Ea_C1;

   sim->params[i].name = "chg_state_fgic";
   sim->params[i].type = "%d";
   sim->params[i++].value= &sim->fgic->ecm->chg_state;

   sim->params[i].name = "prev_chg_state_fgic";
   sim->params[i].type = "%d";
   sim->params[i++].value= &sim->fgic->ecm->prev_chg_state;

   sim->params[i].name = "I_noise_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->I_noise;

   sim->params[i].name = "V_noise_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->V_noise;

   sim->params[i].name = "T_noise_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->T_noise;

   sim->params[i].name = "I_offset_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->I_offset;

   sim->params[i].name = "V_offset_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->V_offset;

   sim->params[i].name = "T_offset_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->T_offset;

   sim->params[i].name = "rest_time_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->rest_time;

   sim->params[i].name = "min_rest_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->min_rest;

   sim->params[i].name = "learning_fgic";
   sim->params[i].type = "%b";
   sim->params[i++].value= &sim->fgic->learning;

   sim->params[i].name = "buf_len_fgic";
   sim->params[i].type = "%d";
   sim->params[i++].value= &sim->fgic->buf_len;

   sim->params[i].name = "dV_max_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->dV_max;

   sim->params[i].name = "dV_min_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->dV_min;

   sim->params[i].name = "dI_max_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->dI_max;

   sim->params[i].name = "dI_min_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->dI_min;

   sim->params[i].name = "V_oc_est_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->V_oc_est;

   sim->params[i].name = "V_oc_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->ecm->V_oc;

   sim->params[i].name = "V_rc_fgic";
   sim->params[i].type = "%lf";
   sim->params[i++].value= &sim->fgic->ecm->V_rc;

   sim->params_sz = i;
   return i;
}




/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn         int check_cond(sim_t *sim, bool *res, char *lop, char *param, char *compare, void *pval)
 *
 *  @brief      Check condition
 *
 *  @param	sim	sim pointer
 *  @param	res	logical result (also input to be && or || with the compare result
 *  @param	lop	logical operator (&& or ||)
 *  @param   	param	parameter name
 *  @param	compare	==, !=, >, >=, <, <= (depending on data type)	
 *  @param	pval	pointer to value register to be compared to
 *
 *  @return	0 if success, negative otherwise
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
int sim_check_cond(sim_t *sim, bool *res, enum LOP lop, char *param, enum LOP compare, double val)
{
   double param_val = 0;
   char *param_type=NULL;
   bool cond = false;

   if (sim==NULL || res==NULL || param==NULL) return -1;

   /* retrieve param type */
   param_type = util_get_params_type(sim, param); 
   if (param_type == NULL || 0 != strcmp(param_type, "%lf")) return -2;

   /* retrieve param value */
   if (util_get_params_val(sim, param, &param_val) < 0) return -3;

   /* compare */
   if (compare == EQ)
      cond = (param_val == val);
   else if (compare == GT)
      cond = (param_val > val);
   else if (compare == GTE)
      cond = (param_val >= val);
   else if (compare == LT)
      cond = (param_val < val);
   else if (compare == LTE)
      cond = (param_val <= val);
   else
      return -4;
   
   /* combine with previous res with lop */
   if (lop == NOP)
      *res = cond;
   else if (lop == AND)
      *res = *res && cond;
   else if (lop == OR)
      *res = *res || cond;
   else
      return -5;

   return 0;
}


/*!
 *----------------------------------------------------------------------------------------------------------------------
 *
 *  @fn         bool sim_check_pause(sim_t *sim)
 *
 *  @brief      Check if simulation should pause
 *
 *  @note       Unprotected
 *
 *----------------------------------------------------------------------------------------------------------------------
 */
static
bool sim_check_pause(sim_t *sim)
{
   bool do_pause = false;
   batt_t *batt = sim->batt;

   /* time-based conditions */
   // if (sim->t >= sim->t_end) do_pause = true;

   /* automatic pause conditions */
   if (batt->ecm->chg_state==CHG && batt->ecm->soc >= 1.0f) do_pause = true;
   if (batt->ecm->chg_state==DSG && batt->ecm->soc <= 0.0f) do_pause = true;

   /* check conditional */
   bool res = false;
   for (int k=0; k<MAX_COND; k++)
   {
      int rc = 0;
      if (sim->cond[k].compare != NOP)
      {
         rc = sim_check_cond(sim, &res, sim->cond[k].lop, sim->cond[k].param, sim->cond[k].compare, sim->cond[k].value);
      }
      if (rc != 0) printf("conditional %d has error.\n", k);
   }

   if (res) 
   {  
      for (int k=0; k<MAX_COND; k++) sim->cond[k].compare = NOP;    /* clear triggered cond so it doesn't repeat */
      do_pause = true;
   }

   return do_pause;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void sim_set_pause(sim_t *sim, bool do_pause)
 *
 *  @brief	Set/unset pause for the simulation
 *
 *  @param	sim:		simulation pointer
 *  @param	do_pause	false: unpause; true: pause
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void sim_set_pause(sim_t *sim, bool do_pause)
{
   LOCK(&sim->mtx); 
   sim->pause = do_pause; 
   UNLOCK(&sim->mtx); 
}

/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn         bool sim_get_pause(sim_t *sim)
 *
 *  @brief      Get simulatlion pause state 
 *
 *  @param      sim:            simulation pointer
 *
 *  @return	true if paused; false otherwise
 *
 *  @note	mutex protected
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
bool sim_get_pause(sim_t *sim)
{
   bool paused = false;
   LOCK(&sim->mtx);
   paused = sim->pause;
   UNLOCK(&sim->mtx);

   return paused;
}



/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn         void timer_callback(void *usr_arg)
 *
 *  @brief      Timer callback for REALTIME runs
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
void timer_callback(void *usr_arg)
{
   sim_t *sim = (sim_t *)usr_arg;
   sim_update(sim);

   if (sim_check_pause(sim))
   {
      itimer_stop(sim->tm);
      printf("run paused at t=%lf (soc_batt=%lf, V_batt=%lf)\n", 
             sim->t, sim->batt->ecm->soc, sim->batt->ecm->V_batt);
   }
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void *sim_loop(void *arg)
 *
 *  @brief	Simulation thread loop
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
void *sim_loop(void *arg)
{
   int rc = 0;
   if (arg==NULL) return NULL;
   sim_t *sim = (sim_t *)arg;

   bool done = sim->done;
   bool pause = sim->pause;  
   while (!done)
   {
      /* check pause */
      LOCK(&sim->mtx); 

      if (sim_check_pause(sim))
         sim->pause = true;

      if (pause && !sim->pause)
         printf("run resumed from t=%lf (soc_batt=%lf, V_batt=%lf)\n", 
             sim->t, sim->batt->ecm->soc, sim->batt->ecm->V_batt);

      pause = sim->pause; 

      if (pause)
         printf("run paused at t=%lf (soc_batt=%lf, V_batt=%lf)\n", 
             sim->t, sim->batt->ecm->soc, sim->batt->ecm->V_batt);

      UNLOCK(&sim->mtx); 

      while (pause)
      {
         LOCK(&sim->mtx); 
         pause = sim->pause; 
         UNLOCK(&sim->mtx); 
         sched_yield();
      }

      /* update sim */
      LOCK(&sim->mtx); 
      done = sim->done;
      if (!done) 
      {	      
	 rc = sim_update(sim);   
      }
      UNLOCK(&sim->mtx); 

      if (rc != 0) goto _err_ret;

      if (done) break;

      sched_yield();
   }
 
   printf("run completed at t=%lf (soc_batt=%lf, V_batt=%lf)\n", 
           sim->t, sim->batt->ecm->soc, sim->batt->ecm->V_batt);
   return NULL;

_err_ret:
   printf("sim_update() error at t=%lf\n", sim->t); 
   return NULL;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		sim_t *sim_create(double t0, double dt, double temp0)
 *
 *  @brief	Create and initialize simulation object
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
sim_t *sim_create(double t0, double dt, double temp0)
{
   sim_t *sim = calloc(1, sizeof(sim_t));
   if (sim == NULL) return NULL;

   sim->logfp = NULL;
   memset(sim->logfn, 0, FN_LEN);
   memset(sim->script_fn, 0, FN_LEN);
   memset(sim->logi, 0, MAX_PARAMS*sizeof(int));
   sim->logn = 0;
   sim->m_root = NULL;

   /* init scope_plot */
   if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) 
   {
      printf("error: cannot init scope.\n");
      goto _err_ret;
   }


   /* init sim */
   sim->t = t0; 
   sim->dt = dt;
   sim->T_amb_C = TEMP_0;

   sim->realtime = false;
   sim->done = false;
   sim->pause = true;

   for (int k=0; k<MAX_COND; k++)
   {
      sim->cond[k].lop = NOP;
      memset(sim->cond[k].param, 0, NAME_LEN*sizeof(char));
      sim->cond[k].compare = NOP;
      sim->cond[k].value = 0;
   }

   sim->batt = batt_create(&g_batt_flash_params, temp0);
   if (sim->batt == NULL) goto _err_ret;

   sim->fgic = fgic_create(sim->batt, &g_fgic_flash_params, temp0);
   if (sim->fgic == NULL) goto _err_ret;

   sim->system = (system_t *)system_create(sim->fgic);
   if (sim->system == NULL) goto _err_ret;

   params_init(sim);

   sim->tm = itimer_create(timer_callback, sim);
   if (sim->tm == NULL) goto _err_ret;

   if (pthread_mutex_init(&sim->mtx, NULL) != 0)
      goto _err_ret;

   sim->thread = (pthread_t *)calloc(1, sizeof(pthread_t));
   if (sim->thread == NULL) goto _err_ret;

   if (pthread_create(sim->thread, NULL, sim_loop, sim) != 0)
      goto _err_ret;

   return sim;


_err_ret:
   if (sim != NULL) sim_destroy(sim);
   return NULL;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int sim_run_start(sim_t *sim)
 *
 *  @brief	Start simulation run
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int sim_run_start(sim_t *sim)
{
   if (sim == NULL) return -1;

   if (sim->realtime)
   {
      return itimer_start(sim->tm, sim->fgic->period);
   }
   else 
   {
      sim_set_pause(sim, false); 
      return 0;
   }
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn         int sim_run_stop(sim_t *sim)
 *
 *  @brief      Stop simulation run
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int sim_run_stop(sim_t *sim)
{
   if (sim == NULL) return -1;

   if (sim->realtime)
      return itimer_stop(sim->tm);
   else
   {
      sim_set_pause(sim, true);
      return 0;
   }
}


/*!
 *----------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void sim_destroy(sim_t *sim)
 *
 *  @brief	Clean up simulation
 *
 *----------------------------------------------------------------------------------------------------------------------
 */
void sim_destroy(sim_t *sim)
{
   if (sim->tm != NULL) itimer_destroy(sim->tm);

   /* kill thread */
   LOCK(&sim->mtx);
   sim->pause = false;
   sim->done = true;
   UNLOCK(&sim->mtx);

   if (sim->thread != NULL) pthread_join(*sim->thread, NULL);
   if (sim->thread != NULL) free(sim->thread);

   if (sim->system != NULL) system_destroy(sim->system);
   if (sim->fgic != NULL) fgic_destroy(sim->fgic);
   if (sim->batt != NULL) batt_destroy(sim->batt);
}



/*!
 *----------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int sim_update_log(sim_t *sim)
 *
 *  @brief	Update logging
 *
 *----------------------------------------------------------------------------------------------------------------------
 */
static
int sim_update_log(sim_t *sim)
{
   char fmt[200];
   memset(fmt, 0, sizeof(fmt));

   if (sim != NULL && sim->logn > 0)
   {
      fprintf(sim->logfp, "%lf ", sim->t); 
      for (int i=0; i<sim->logn; i++)
      {
         int idx = sim->logi[i];
         char *type = sim->params[idx].type;
         if (i == sim->logn-1)
            snprintf(fmt, sizeof(fmt), "%s", type);
         else
            snprintf(fmt, sizeof(fmt), "%s ", type);

         if (0==strcmp(type, "%d")) 
            fprintf(sim->logfp, fmt, *(int *)sim->params[idx].value); 
         else if (0==strcmp(type, "%ld")) 
            fprintf(sim->logfp, fmt, *(long *)sim->params[idx].value); 
         else if (0==strcmp(type, "%f")) 
            fprintf(sim->logfp, fmt, *(float *)sim->params[idx].value); 
         else if (0==strcmp(type, "%lf")) 
            fprintf(sim->logfp, fmt, *(double *)sim->params[idx].value); 
         else 
            fprintf(sim->logfp, "%s", (char *)sim->params[idx].value); 
      }
      fprintf(sim->logfp, "\n");
   }
   return 0;
}


/*!
 *----------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int sim_update(sim_t *sim)
 *
 *  @brief	Perform simulation update one time step
 *
 *----------------------------------------------------------------------------------------------------------------------
 */
int sim_update(sim_t *sim)
{
   int rc = 0;

   if (sim == NULL) return -1;

   sim_update_log(sim);
   rc = system_update(sim->system, sim->t, sim->dt);
   if (rc != 0) goto _err_ret;

   rc = batt_update(sim->batt, sim->system->I, sim->T_amb_C, sim->t, sim->dt);
   if (rc != 0) goto _err_ret;

   rc = fgic_update(sim->fgic, sim->T_amb_C, sim->t, sim->dt);
   if (rc != 0) goto _err_ret;

   sim->t += sim->dt;
   return 0;

_err_ret:
   printf("error: sim_update.\n");
   return rc; 
}


