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
 *  @fn		void fgic_fx(double *x, const double *u, double dt, void *p_usr)
 *
 *  @brief	UKF process model updates the state vector 'x' given 'u'
 *
 *  @param	x:		state vector x[0]=soc, x[1] = V_rc
 *  @param	u:		input vector u[0]=dq,  u[1] = T_amb_C
 *  @param	p_usr:	 	pass in fgic pointer	
 *
 *  @note	'dq' is delta charge from coulomb counter.  If current is measured, let dq=I*dt 
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
void fgic_fx(double *x, const double *u, double dt, void *p_usr)
{
   fgic_t *fgic = (fgic_t *)p_usr;
   ecm_t *ecm = fgic->ecm;

   double R0=0, R1=0, C1=0;

   /* read state vars */
   ecm->soc  = x[0];
   ecm->V_rc = x[1];
   ecm->T_C  = x[2];

   /* read inputs */
   ecm->I       = u[0];
   ecm->T_amb_C = u[1];
   
   /* update SOC */
   double Qmax = ecm->Q_Ah * 3600;
   ecm->soc -= (ecm->I*dt)/Qmax;
   
   /* lookup R0, R1, C1 */
   ecm_lookup_r0(ecm, util_clamp(ecm->soc, 0.0, 1.0), &R0);
   ecm->R0 = util_temp_adj(R0, ecm->Ea_R0, ecm->T_C, ecm->params->T_ref_C);
   ecm_lookup_r1(ecm, util_clamp(ecm->soc, 0.0, 1.0), &R1);
   ecm->R1 = util_temp_adj(R1, ecm->Ea_R1, ecm->T_C, ecm->params->T_ref_C);
   ecm_lookup_c1(ecm, util_clamp(ecm->soc, 0.0, 1.0), &C1);
   ecm->C1 = util_temp_adj(C1, ecm->Ea_C1, ecm->T_C, ecm->params->T_ref_C);

   /* update VRC */
   double tau = ecm->R1 * ecm->C1; 
   if (tau < 1e-9) tau = 1e-9;
   ecm->V_rc += dt * (-ecm->V_rc / tau + ecm->I / ecm->C1);

   /* update T */
   double powerloss = ecm->I * ecm->I * ecm->R0;
   ecm->T_C += dt * (powerloss - ecm->ht * (ecm->T_C - ecm->T_amb_C)) / ecm->Cp;


   /* update state vars */
   x[0] = ecm->soc;
   x[1] = ecm->V_rc;
   x[2] = ecm->T_C;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void fgic_hx(const double *x, double *z, void *p_usr)
 *
 *  @brief	measurement model--compute expected measurement 'z' given the state vector 'x' 
 *
 *  @paaram	x:	state vector
 *        		x[0] = soc
 *        		x[1] = V_rc 
 *        		x[2] = T 
 *
 *  @param	z:	measurement vector
 *        		z[0] = V_term 
 *        		z[1] = T 
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
void fgic_hx(const double *x, double *z, void *p_usr)
{
   fgic_t *fgic = (fgic_t *)p_usr;
   ecm_t *ecm = fgic->ecm;

   double soc  = x[0];
   double V_rc = x[1];
   double T_C  = x[2];

   ecm_lookup_ocv(ecm, util_clamp(soc, 0.0, 1.0), &ecm->V_oc);

   /* Track charging state for hysteresis sign */
   ecm->prev_chg_state = ecm->chg_state;

   /* update chg state -- must be before update H */
   if (ecm->I > ecm->I_quit)
      ecm->chg_state = DSG;
   else if (ecm->I < -ecm->I_quit)
      ecm->chg_state = CHG;
   else
      ecm->chg_state = REST;

   /* Update H */
   ecm_lookup_h(ecm, util_clamp(soc, 0.0, 1.0), &ecm->H);
  
   /* compute V_term and T_C */ 
   z[0] = (ecm->V_oc + ecm->H) - V_rc - ecm->I * ecm->R0;
   z[1] = T_C;
}


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
   if (ecm_init(fgic->ecm, &g_flash_params, T0_C)!=0) 
      goto _err_ret;

   fgic->I_meas = batt->ecm->I;
   fgic->V_meas = batt->ecm->V_batt;
   fgic->T_meas = batt->ecm->T_C;


   /* UKF setup */
   const int n_x = 3;	/* SOC, VRC, T */
   const int n_z = 2;	/* V, T */

   fgic->ukf = (ukf_t *)calloc(1, sizeof(ukf_t));
  
   double alpha = 1e-3;
   double beta  = 2.0;
   double kappa = 0.0;

   /* initial states and covariance */
   double x0[3]; 
   double P0[9] = {
      0.2, 0.0, 0.0,
      0.0, 0.5, 0.0,
      0.0, 0.0, 1.0
   }; 

   /* process noise */
   double q_soc = 1e-4;
   double q_vrc = 1e-4;
   double q_T = 1e-4;
   double Q[9] = {
      q_soc,   0.0,   0.0,
        0.0, q_vrc,   0.0,
        0.0,   0.0,   q_T
   };

   /* measurement noise */
   double R_meas = fgic->V_noise * fgic->V_noise;
   double T_meas = fgic->T_noise * fgic->T_noise;
   double R[4] = { 
      R_meas,    0.0,
         0.0, T_meas
   };

   /* init ukf */
   if (ukf_init(fgic->ukf, n_x, n_z, alpha, beta, kappa) != UKF_OK)
      goto _err_ret;
   

   /* set fx and hx */
   ukf_set_models(fgic->ukf, fgic_fx, fgic_hx);


   /* set state */
   if (ukf_set_state(fgic->ukf, x0, P0) != UKF_OK) 
      goto _err_ret;


   /* set noise */
   if (ukf_set_noise(fgic->ukf, Q, R) != UKF_OK) 
      goto _err_ret;

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
 *  @fn		int fgic_update(fgic_t *fgic, double T_amb_C, double t, double dt)
 *
 *  @brief	Update FGIC one time step
 *
 *  @return	0 if success; negative otherwise
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int fgic_update(fgic_t *fgic, double T_amb_C, double t, double dt)
{
   (void)t;
   (void)dt;

   if (fgic == NULL || fgic->ecm == NULL) goto _err_ret;
 
   /* measure battery I, T, V with noise */
   fgic->I_meas = fgic->batt->ecm->I + fgic->I_noise * ((double)rand()/(double)RAND_MAX-0.5);
   fgic->T_meas = fgic->batt->ecm->T_C + fgic->T_noise * ((double)rand()/(double)RAND_MAX-0.5);
   fgic->V_meas = fgic->batt->ecm->V_batt + fgic->V_noise * ((double)rand()/(double)RAND_MAX-0.5);

   double z_meas[2];
   z_meas[0] = fgic->V_meas;
   z_meas[1] = fgic->T_meas;

   double u[2];
   u[0] = fgic->I_meas;
   u[1] = T_amb_C;

#if 1
   /* ukf_predict() calls process_model() */
   if (ukf_predict(fgic->ukf, u, dt, (void *)fgic) != UKF_OK) goto _err_ret;
    
   /* ukf_update() calls  measure_model() */
   if (ukf_update(fgic->ukf, z_meas, (void *)fgic) != UKF_OK) goto _err_ret;
#endif
   return 0;

_err_ret:
   return -1;
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
   if (fgic == NULL) return;

   if (fgic->ecm != NULL) 
   {
      ecm_cleanup(fgic->ecm);
      free(fgic->ecm);
   }
   if (fgic->ukf != NULL) free(fgic->ukf);
   if (fgic != NULL) free(fgic);
}

