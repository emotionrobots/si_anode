/*!
 *=====================================================================================================================
 *
 * @file		ecm.c
 *
 * @brief		ECM implementation
 *
 *=====================================================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "util.h"
#include "ecm.h"


/*! 
 *--------------------------------------------------------------------------------------------------------------------- 
 *
 * @fn 		int tbl_interp(const double *grid, const double *tbl, int n, double soc, double *val)
 *
 * @brief	Linear interpolation on SOC grid (assumed monotonic). 
 *
 * @return	0 if success; negative otherwise
 *
 *--------------------------------------------------------------------------------------------------------------------- 
 */
static 
int tbl_interp(const double *grid, const double *tbl, int n, double soc, double *val)
{
    if (soc <= grid[0]) 
    {
       *val = tbl[0];
    }
    else if (soc >= grid[n - 1]) 
    {
       *val = tbl[n - 1];
    }
    else 
    {
       for (int i = 0; i < n - 1; ++i) 
       {
           double s0 = grid[i];
           double s1 = grid[i + 1];
           if (soc >= s0 && soc <= s1) 
   	   {
               double t = (soc - s0) / (s1 - s0);
               *val = tbl[i] + t * (tbl[i + 1] - tbl[i]);
	       return 0;
           }
       }
    }

    /* Fallback (should not happen): */
    return -1;
}



/*! 
 *--------------------------------------------------------------------------------------------------------------------- 
 *
 * @fn		void ecm_init_default(ecm_t *ecm, flash_params_t *p, double T0_C)
 *
 * @brief 	Initialize ECM as a full cell. 
 *
 * @note      	Sets reasonable defaults for OCV, R0, R1, C1, hysteresis, Arrhenius, thermal parameters, 
 *       	and initial state.
 *
 *--------------------------------------------------------------------------------------------------------------------- 
 */
int ecm_init(ecm_t *ecm, flash_params_t *p, double T0_C)
{
   if (ecm == NULL || p == NULL) return -1;

   memset(ecm, 0, sizeof(*ecm));

   /* init g_flash_params for fgic */
   for (int i=0; i<SOC_GRIDS; i++) ecm->params.soc_tbl[i] = p->soc_tbl[i];
   for (int i=0; i<SOC_GRIDS; i++) ecm->params.ocv_tbl[i] = p->ocv_tbl[i];
   for (int i=0; i<SOC_GRIDS; i++) ecm->params.r0_tbl[i] = p->r0_tbl[i];
   for (int i=0; i<SOC_GRIDS; i++) ecm->params.r1_tbl[i] = p->r1_tbl[i];
   for (int i=0; i<SOC_GRIDS; i++) ecm->params.c1_tbl[i] = p->c1_tbl[i];
   for (int i=0; i<SOC_GRIDS; i++) ecm->params.h_chg_tbl[i] = p->h_chg_tbl[i];
   for (int i=0; i<SOC_GRIDS; i++) ecm->params.h_dsg_tbl[i] = p->h_dsg_tbl[i];
   for (int i=0; i<SOC_GRIDS; i++) ecm->params.h_dsg_tbl[i] = p->h_dsg_tbl[i];

   ecm->params.design_capacity = p->design_capacity;
   ecm->params.v_end = p->v_end;
   ecm->params.T_ref_C = p->T_ref_C;

   ecm->params.I_quit = p->I_quit;
   ecm->params.I_chg = p->I_chg;
   ecm->params.V_chg = p->V_chg;

   ecm->I_quit = p->I_quit;

   /* Arrhenius parameters (example values) */
   ecm->Ea_R0 = DEFAULT_EA_R0; 
   ecm->Ea_R1 = DEFAULT_EA_R1;
   ecm->Ea_C1 = DEFAULT_EA_C1;

   /* Capacity and thermal */
   ecm->Q_Ah = Q_DESIGN; 	/* Ah */
   ecm->Cp = HEAT_CAPACITY;    	/* Thermal capacity J/°C */
   ecm->ht = HEAT_TRANS_COEF;  	/* Thermal resistance °C/W */

   /* Dynamic state defaults */
   ecm->soc = ecm->params.soc_tbl[SOC_GRIDS-1];
   ecm->V_oc = ecm->params.ocv_tbl[SOC_GRIDS-1];
   ecm->V_rc = 0.0;
   ecm->V_batt = ecm->V_oc;
   ecm->H = ecm->params.h_dsg_tbl[SOC_GRIDS-1];
   ecm->prev_V_batt = ecm->V_batt;
   ecm->prev_V_rc = ecm->V_rc;
   ecm->prev_V_oc = ecm->V_oc;
   ecm->prev_H = ecm->H;
   ecm->prev_I = 0.0;
   ecm->I = ecm->prev_I;
   ecm->T_C = T0_C;

   /* R0, R1, C1 */
   ecm_lookup_r0(ecm, ecm->soc, &ecm->R0);
   ecm_lookup_r1(ecm, ecm->soc, &ecm->R1);
   ecm_lookup_c1(ecm, ecm->soc, &ecm->C1);
   
   ecm->Tau = ecm->C1 * ecm->R1;

   ecm->prev_chg_state = REST;
   ecm->chg_state = ecm->prev_chg_state;

   memset(ecm->Vrc_buf, 0, VRC_BUF_SZ*sizeof(double));
   ecm->Vrc_buf_len = 0;

   return 0;
}



/*! 
 *--------------------------------------------------------------------------------------------------------------------- 
 *
 *  @fn		void ecm_cleanup(ecm_t *ecm)
 *
 *  @brief	Cleanup ECM
 *
 *--------------------------------------------------------------------------------------------------------------------- 
 */
void ecm_cleanup(ecm_t *ecm)
{
   (void)ecm;
}


/*! 
 *--------------------------------------------------------------------------------------------------------------------- 
 *
 *  @fn		int ecm_lookup_ocv(const ecm_t *ecm, double soc, double T, double *val)
 *
 *  @brief	Read OCV table given SOC
 *
 *--------------------------------------------------------------------------------------------------------------------- 
 */
int ecm_lookup_ocv(const ecm_t *ecm, double soc, double *val)
{
    return tbl_interp(ecm->params.soc_tbl, ecm->params.ocv_tbl, SOC_GRIDS, soc, val);
}


/*! 
 *--------------------------------------------------------------------------------------------------------------------- 
 *
 *  @fn		int ecm_lookup_h(const ecm_t *ecm, double soc, double *val)
 *
 *  @brief	Read  H lookup given SOC
 *
 *--------------------------------------------------------------------------------------------------------------------- 
 */
int ecm_lookup_h(const ecm_t *ecm, double soc, double *val)
{
    int rc=0;
    double H=0;

    if (ecm->chg_state==CHG)
    {
       rc = tbl_interp(ecm->params.soc_tbl, ecm->params.h_chg_tbl, SOC_GRIDS, soc, &H);
       *val = H;
    }
    else if (ecm->chg_state==DSG) 
    {
       rc = tbl_interp(ecm->params.soc_tbl, ecm->params.h_dsg_tbl, SOC_GRIDS, soc, &H);
       *val = H;
    }
    else
       *val = ecm->H;   // no change

    return rc;
}


/*! 
 *--------------------------------------------------------------------------------------------------------------------- 
 *
 * @fn		int ecm_lookup_r0(const ecm_t *ecm, double soc, double *r0_val)
 *
 * @brief	Read R0 given SOC 
 *
 *--------------------------------------------------------------------------------------------------------------------- 
 */
int ecm_lookup_r0(const ecm_t *ecm, double soc, double *val)
{
    return tbl_interp(ecm->params.soc_tbl, ecm->params.r0_tbl, SOC_GRIDS, soc, val);
}

/*! 
 *--------------------------------------------------------------------------------------------------------------------- 
 *
 * @fn		int ecm_lookup_r1(const ecm_t *ecm, double soc, double *val)
 *
 * @brief	Read R1 given SOC
 *
 *--------------------------------------------------------------------------------------------------------------------- 
 */
int ecm_lookup_r1(const ecm_t *ecm, double soc, double *val)
{
    return tbl_interp(ecm->params.soc_tbl, ecm->params.r1_tbl, SOC_GRIDS, soc, val);
}


/*! 
 *--------------------------------------------------------------------------------------------------------------------- 
 *
 *  @fn		int ecm_lookup_c1(const ecm_t *ecm, double soc, double *val)
 *
 *  @brief	Read C1 given SOC
 *
 *--------------------------------------------------------------------------------------------------------------------- 
 */
int ecm_lookup_c1(const ecm_t *ecm, double soc, double *val)
{
    return tbl_interp(ecm->params.soc_tbl, ecm->params.c1_tbl, SOC_GRIDS, soc, val);
}



/*! 
 *--------------------------------------------------------------------------------------------------------------------- 
 *
 * @fn		int ecm_update(ecm_t *ecm, double I, double T_amb_C, double t, double dt)
 * 
 * @brief	Compute the ECM dynamics one time step
 *
 * @param	ecm   : ECM instance
 * @param	I     : cell current [A], I > 0 = discharge, I < 0 = charge
 * @param	T     : cell temperature [°C]
 * @param	dt    : time step [s]
 *
 * @note	States updated: soc, V_rc, T, chg_state
 * 		Parameters R0/R1/C1 are read via lookup + Arrhenius scaling.
 *
 *--------------------------------------------------------------------------------------------------------------------- 
 */
int ecm_update(ecm_t *ecm, double I, double T_amb_C, double t, double dt)
{
    (void)t;


    double R0=0.0, C1=0.0, R1=0.0;
    int rc = 0;


    ecm->prev_I = ecm->I;
    ecm->prev_V_batt = ecm->V_batt;
    ecm->prev_V_rc = ecm->V_rc;
    ecm->prev_V_oc = ecm->V_oc;
    ecm->prev_H = ecm->H;

    ecm->I = I;
    ecm->T_amb_C = T_amb_C;
   
    /* soc update */
    double Qmax = ecm->Q_Ah * 3600;
    ecm->soc -= (ecm->I*dt)/Qmax;
    ecm->soc = util_clamp(ecm->soc, 0.0, 1.0);

    /* model param update */
    ecm_lookup_r0(ecm, ecm->soc, &R0);
    ecm->R0 = util_temp_adj(R0, ecm->Ea_R0, ecm->T_C, ecm->params.T_ref_C);

    ecm_lookup_r1(ecm, ecm->soc, &R1);
    ecm->R1 = util_temp_adj(R1, ecm->Ea_R1, ecm->T_C, ecm->params.T_ref_C); 

    ecm_lookup_c1(ecm, ecm->soc, &C1);
    ecm->C1 = util_temp_adj(C1, ecm->Ea_C1, ecm->T_C, ecm->params.T_ref_C);  

    /* update Tau */
    ecm->Tau = ecm->C1 * ecm->R1;

    /* update V_oc */
    ecm_lookup_ocv(ecm, ecm->soc, &ecm->V_oc);
    

    /* update V_rc */
    ecm->V_rc += dt * ( -ecm->V_rc / (ecm->R1 * ecm->C1) + ecm->I / ecm->C1 );

    /* temp update */
    double powerloss = ecm->I * ecm->I * ecm->R0;
    ecm->T_C += dt * (powerloss - ecm->ht * (ecm->T_C - ecm->T_amb_C)) / ecm->Cp;


    ecm->prev_chg_state = ecm->chg_state;
    
    /* update chg state -- must be before update H */
    if (ecm->I > ecm->I_quit)
       ecm->chg_state = DSG; 		
    else if (ecm->I < -ecm->I_quit)
       ecm->chg_state = CHG; 		
    else
       ecm->chg_state = REST; 


    /* update H */
    ecm_lookup_h(ecm, ecm->soc, &ecm->H);


    /* update V_batt */
    ecm->V_batt = (ecm->V_oc + ecm->H) - ecm->V_rc - ecm->I * ecm->R0;


    return rc;
}


/*! 
 *--------------------------------------------------------------------------------------------------------------------- 
 *
 *  @fn		void ecm_update_delta(ecm_t *ecm)
 * 
 *  @brief	update previous values 
 *
 *--------------------------------------------------------------------------------------------------------------------- 
 */
void ecm_update_delta(ecm_t *ecm)
{
    ecm->prev_I = ecm->I;
    ecm->prev_V_batt = ecm->V_batt;
    ecm->prev_V_rc = ecm->V_rc;
}

