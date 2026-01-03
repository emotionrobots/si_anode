/*!
 *=====================================================================================================================
 *
 *  @file		fgic.c
 *
 *  @brief		FGIC object implementation 
 *
 *=====================================================================================================================
 */
#include "fgic.h"

extern flash_params_t g_flash_params;


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		fgic_t *fgic_create(batt_t *batt, flash_param_t *p, double T0_C)
 *
 *  @brief	Create and init FGIC
 *
 *  @return	0 if success; negative otherwise
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
fgic_t *fgic_create(batt_t *batt, flash_params_t *p, double T0_C)
{
   if (batt==NULL || p==NULL) return NULL;

   fgic_t *fgic = calloc(1, sizeof(fgic_t));

   if (fgic==NULL) goto _err_ret;
	   
   fgic->params = p;
   fgic->I_quit = DEFAULT_I_QUIT;
   fgic->V_chg = DEFAULT_CV;
   fgic->I_chg = DEFAULT_CC;
   fgic->V_noise = DEFAULT_V_NOISE;
   fgic->I_noise = DEFAULT_I_NOISE;
   fgic->T_noise = DEFAULT_T_NOISE;
   fgic->V_offset = DEFAULT_V_OFFSET;
   fgic->I_offset = DEFAULT_I_OFFSET;
   fgic->T_offset = DEFAULT_T_OFFSET;
   fgic->period = FGIC_PERIOD_MS;
   fgic->batt = batt;

   fgic->ecm = (ecm_t *)malloc(sizeof(ecm_t));
   if (ecm_init(fgic->ecm, &g_flash_params, T0_C)==0) 
      return fgic;

_err_ret: 
   fgic_destroy(fgic);
   return NULL;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int fgic_get_cccv(fgic_t *fgic, double *cc, double *cv)
 *
 *  @brief	Read back CC and CV
 *
 *  @return	0 if success; negative otherwise
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int fgic_get_cccv(fgic_t *fgic, double *cc, double *cv)
{
   if (fgic == NULL || cc == NULL || cv == NULL) return -1;

   *cc = fgic->I_chg;
   *cv = fgic->V_chg;

   return 0;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int fgic_read_ivt(fgic_t *fgic)
 *
 *  @brief	Read battery IVT with measurement noise and offset
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
int fgic_read_ivt(fgic_t *fgic)
{
   if (fgic != NULL && fgic->batt != NULL) 
   {
      batt_t *batt = fgic->batt;
      double V_noise = fgic->V_noise * ((double)rand()/(double)RAND_MAX-0.5); 
      double I_noise = fgic->I_noise * ((double)rand()/(double)RAND_MAX-0.5); 
      double T_noise = fgic->T_noise * ((double)rand()/(double)RAND_MAX-0.5); 

      fgic->ecm->v_batt = batt->ecm->v_batt + V_noise + fgic->V_offset; 
      fgic->ecm->I      = batt->ecm->I + I_noise + fgic->I_offset; 
      fgic->ecm->T_C    = batt->ecm->T_C + T_noise + fgic->T_offset; 
      return 0;
   } 
   return -1;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int fgic_update(fgic_t *fgic, double I, double T, double t, double dt)
 *
 *  @brief	Update FGIC one time step
 *
 *  @return	0 if success; negative otherwise
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int fgic_update(fgic_t *fgic, double t, double dt)
{
   int rc = 0;

   if (fgic == NULL || fgic->ecm == NULL) return -1;

   if (fgic_read_ivt(fgic) != 0)
      return -1;
#if 0
   - Read battery I, V, T measurements 
   - Collect Vrc_buffer
   
   - Apply UKF to find best SOC/Vrc estimates
   - Calc V_batt 

   - Extract R0 from dV/dI if condition is right
   - Update R1 C1 from Vrc_buffer
#endif

   return rc;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void fgic_destroy(fgic_t *fgic)
 *
 *  @brief	Clean-up FGIC
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void fgic_destroy(fgic_t *fgic)
{
   if (fgic->ecm != NULL) 
   {
      ecm_cleanup(fgic->ecm);
      free(fgic->ecm);
   }
   if (fgic != NULL) free(fgic);
}

