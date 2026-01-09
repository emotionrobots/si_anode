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


   /* read state vars */
   double soc  = x[0];
   double V_rc = x[1];
   double T_C  = x[2];

   /* read inputs */
   double I       = u[0];
   double T_amb_C = u[1];
 
   /* update soc */ 
   double Qmax = ecm->Q_Ah * 3600;
   soc -= (I*dt)/Qmax;   
   soc = util_clamp(soc, 0.0, 1.0);

   /* lookup R0, R1, C1 */
   double R0=0, R1=0, C1=0;
   ecm_lookup_r0(ecm, soc, &R0);
   R0 = util_temp_adj(R0, ecm->Ea_R0, ecm->T_C, ecm->params->T_ref_C);
   ecm_lookup_r1(ecm, soc, &R1);
   R1 = util_temp_adj(R1, ecm->Ea_R1, ecm->T_C, ecm->params->T_ref_C);
   ecm_lookup_c1(ecm, soc, &C1);
   C1 = util_temp_adj(C1, ecm->Ea_C1, ecm->T_C, ecm->params->T_ref_C);

   double tau = R1 * C1; 
   if (tau < 1e-9) tau = 1e-9;

   /* update VRC */
   V_rc += dt * (-V_rc / tau + I / C1);

   /* update T */
   double powerloss = I * I * R0;
   T_C += dt * (powerloss - ecm->ht * (T_C - T_amb_C)) / ecm->Cp;

   /* update state vars */
   x[0] = soc;
   x[1] = V_rc;
   x[2] = T_C;
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

   double soc  = util_clamp(x[0], 0.0, 1.0);
   double V_rc = x[1];
   double T_C  = x[2];

   double V_oc;
   ecm_lookup_ocv(ecm, soc, &V_oc);

   /* Update H */
   double H;
   ecm_lookup_h(ecm, soc, &H);

   double R0;
   ecm_lookup_r0(ecm, soc, &R0);
   R0 = util_temp_adj(R0, ecm->Ea_R0, T_C, ecm->params->T_ref_C);   

   /* compute V_term and T_C */ 
   z[0] = (V_oc + H) - V_rc - ecm->I * R0;
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
   fgic->V_chg = DEFAULT_CV;
   fgic->I_chg = DEFAULT_CC;
   fgic->V_noise = DEFAULT_V_NOISE;
   fgic->I_noise = DEFAULT_I_NOISE;
   fgic->T_noise = DEFAULT_T_NOISE;
   fgic->V_offset = DEFAULT_V_OFFSET;
   fgic->I_offset = DEFAULT_I_OFFSET;
   fgic->T_offset = DEFAULT_T_OFFSET;
   fgic->period = FGIC_PERIOD_MS;
   fgic->min_rest = MIN_REST_TIME;
   fgic->rest_time = 0.0;
   fgic->learning = false;
   fgic->buf_len = 0;

   fgic->batt = batt;

   fgic->ecm = (ecm_t *)malloc(sizeof(ecm_t));
   if (ecm_init(fgic->ecm, p, T0_C) != 0) goto _err_ret;
   fgic->ecm->soc = 0.5;  // set wrong initially
   fgic->ecm->prev_V = fgic->ecm->V_batt;
   fgic->ecm->prev_I = fgic->ecm->I;

   fgic->I_meas = batt->ecm->I + fgic->I_noise * ((double)rand()/(double)RAND_MAX-0.5);
   fgic->T_meas = batt->ecm->T_C + fgic->T_noise * ((double)rand()/(double)RAND_MAX-0.5);
   fgic->V_meas = batt->ecm->V_batt + fgic->V_noise * ((double)rand()/(double)RAND_MAX-0.5);
   fgic->ecm->T_C = fgic->T_meas; 

   /* UKF setup */
   const int n_x = 3;	/* SOC, VRC, T */
   const int n_z = 2;	/* V, T */

   fgic->ukf = (ukf_t *)calloc(1, sizeof(ukf_t));
  
   double alpha = 1e-3;
   double beta  = 2.0;
   double kappa = 0.0;

   /* initial states and covariance */
   double x0[3]; 
   x0[0] = fgic->ecm->soc;
   x0[1] = fgic->ecm->V_rc;
   x0[2] = fgic->ecm->T_C;

   double P0[9] = {
      0.01, 0.0, 0.0,
      0.0,  0.5, 0.0,
      0.0,  0.0, 1.0
   }; 

   /* process noise */
   double q_soc = 1e-4;
   double q_vrc = 1e-4;
   double q_T   = 1e-4;
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


   if (fgic == NULL || fgic->ecm == NULL) goto _err_ret;
   ecm_t *ecm = fgic->ecm;


   /* measure battery I, T, V with noise */
   fgic->I_meas = fgic->batt->ecm->I + fgic->I_noise * ((double)rand()/(double)RAND_MAX-0.5);
   fgic->T_meas = fgic->batt->ecm->T_C + fgic->T_noise * ((double)rand()/(double)RAND_MAX-0.5);
   fgic->V_meas = fgic->batt->ecm->V_batt + fgic->V_noise * ((double)rand()/(double)RAND_MAX-0.5);

   ecm->I = fgic->I_meas;
   ecm->T_amb_C = T_amb_C;
   ecm->V_batt = fgic->V_meas;


   double z_meas[2];
   z_meas[0] = fgic->V_meas;
   z_meas[1] = fgic->T_meas;

   double u[2];
   u[0] = ecm->I;
   u[1] = T_amb_C;



   /* Predict x given u */
   if (ukf_predict(fgic->ukf, u, dt, (void *)fgic) != UKF_OK) goto _err_ret;
   /* Update x given z_meas */
   if (ukf_update(fgic->ukf, z_meas, (void *)fgic) != UKF_OK) goto _err_ret;


   /* update SOC from ukf */
   double soc  = util_clamp(fgic->ukf->x[0], 0.0, 1.0);


   /* update V_rc */
   ecm->V_rc = fgic->ukf->x[1];


   /* update T_C */
   ecm->T_C  = fgic->ukf->x[2];


   /* update R0, R1, C1 */
   double R0, R1, C1;
   ecm_lookup_r0(ecm, soc, &R0);
   ecm->R0 = util_temp_adj(R0, ecm->Ea_R0, ecm->T_C, ecm->params->T_ref_C);
   ecm_lookup_r1(ecm, soc, &R1);
   ecm->R1 = util_temp_adj(R1, ecm->Ea_R1, ecm->T_C, ecm->params->T_ref_C);
   ecm_lookup_c1(ecm, soc, &C1);
   ecm->C1 = util_temp_adj(C1, ecm->Ea_C1, ecm->T_C, ecm->params->T_ref_C);


   /* update H */
   ecm_lookup_h(ecm, soc, &ecm->H);


   /* update V_oc */
   ecm->V_oc = ecm->V_batt - ecm->H + ecm->V_rc + ecm->I*ecm->R0;


   /* update soc based on OCV lookup if rest time long enough */
   if (fgic->rest_time >= fgic->min_rest) 
   {
      ecm->soc = soc_from_ocv_best(ecm->V_oc, soc, ecm->chg_state, ecm->params);
   }
   else
   {
      ecm->soc = soc;
   }


#if 1
   //-----------------------------------------------------------------------------
   //  Perform model learning/update to track H, R0, R1, C1
   //-----------------------------------------------------------------------------
  
   /* Collect data if at REST */
   if (ecm->chg_state == REST)
   {
      double R0_est=0, R1_est=0, C1_est=0; 
      double R0_ref=0, R1_ref=0, C1_ref=0; 
      double ratio;

      /* estimate R0 from initial dV from CHG->REST or DSG->REST by R0 = dV/dI */
      if (ecm->prev_chg_state != REST)
      {
         double dV = ecm->V_batt - fgic->ecm->prev_V;
	 double dI = ecm->I - fgic->ecm->prev_I;

	 /* temperature adjust R0 and update R0 table at current soc */
	 R0_est = util_temp_unadj(fabs(dV/dI), ecm->Ea_R0, ecm->T_C, ecm->params->T_ref_C);
         ecm_lookup_r0(ecm, ecm->soc, &R0_ref);
         ratio = R0_est / R0_ref;
         for (int k=0; k<SOC_GRIDS; k++)
            ecm->params->r0_tbl[k] *= ratio; 

	 /* clear vrc_buf */
         fgic->buf_len = 0; 
	 fgic->learning = true;
      }

      if (fgic->learning) 
      {
	 if (fgic->buf_len < VRC_BUF_SZ)    	/* record VRC data if not full */ 
	 {
	    fgic->vrc_buf[fgic->buf_len].i = ecm->I;
	    fgic->vrc_buf[fgic->buf_len].v = ecm->V_batt - ecm->H - ecm->I * R0_est;
	    fgic->buf_len++;
         }
         else 					/* is full and learning, learn the parameters */ 
         {
            /* Fit dV_rc/dt = -a*Vrc + b*I (a=1/(R1*C1), b=1/C1) */
	    double Sa=0, Sb=0, Sab=0, Sya=0, Syb=0;
            for (int k=0; k<VRC_BUF_SZ-1; k++)
	    {
               double dvrc = (fgic->vrc_buf[k+1].v - fgic->vrc_buf[k].v) / dt;
	       double x1 = fgic->vrc_buf[k].v;
	       double x2 = fgic->vrc_buf[k].i;
               Sa += x1*x1;
	       Sb += x2*x2;
	       Sab += x1*x2;
	       Sya += x1*dvrc;
	       Syb += x2*dvrc;
	    } 
	    double det = Sa*Sb - Sab*Sab;
	    double a = (Sb*Sya - Sab*Syb) / (det + 1e-12);
	    double b = (-Sab*Sya + Sa*Syb) / (det + 1e-12);

	    C1_est = 1.0/(b+1e-12);
	    R1_est = fabs(1.0/(a*C1_est+1e-12));

	    /* update C1 table */
	    C1_est = util_temp_unadj(C1_est, ecm->Ea_C1, ecm->T_C, ecm->params->T_ref_C);
            ecm_lookup_c1(ecm, ecm->soc, &C1_ref);
            ratio = C1_est / C1_ref;
            for (int k=0; k<SOC_GRIDS; k++)
               ecm->params->c1_tbl[k] *= ratio; 

	    /* update R1 table */
	    R1_est = util_temp_unadj(R1_est, ecm->Ea_R1, ecm->T_C, ecm->params->T_ref_C);
            ecm_lookup_r1(ecm, ecm->soc, &R1_ref);
            ratio = R1_est / R1_ref;
            for (int k=0; k<SOC_GRIDS; k++)
               ecm->params->r1_tbl[k] *= ratio; 

	    fgic->learning = false;
	    printf("learned...\n");
         }
      }
   }
#endif  

   //-----------------------------------------------------------------------------
   //  Update charging state 
   //-----------------------------------------------------------------------------
   fgic->ecm->prev_V = ecm->V_batt;
   fgic->ecm->prev_I = ecm->I;

   ecm->prev_chg_state = ecm->chg_state;

   /* update chg state -- must be before update H */
   if (ecm->I > ecm->I_quit)
      ecm->chg_state = DSG;
   else if (ecm->I < -ecm->I_quit)
      ecm->chg_state = CHG;
   else
      ecm->chg_state = REST;

   /* check if fgic->rest_time needs to reset */
   if (ecm->chg_state != REST )
      fgic->rest_time = 0.0;
   else if (fgic->rest_time < fgic->min_rest)
      fgic->rest_time += dt;


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

