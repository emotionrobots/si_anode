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
 * @fn		int nearest_soc_idx(const double *grid, int n, double soc)
 *
 * @brief	Find nearest SOC bin index. 
 *
 *--------------------------------------------------------------------------------------------------------------------- 
 */
static 
int nearest_soc_idx(const double *grid, int n, double soc)
{
    soc = util_clamp(soc, grid[0], grid[n - 1]);
    int best = 0;
    double best_err = fabs(soc - grid[0]);
    for (int i = 1; i < n; ++i) 
    {
        double e = fabs(soc - grid[i]);
        if (e < best_err) 
	{
            best_err = e;
            best = i;
        }
    }
    return best;
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
   ecm->params = p;

   ecm->I_quit = p->I_quit;

   /* Arrhenius parameters (example values) */
   ecm->Ea_R0 =  0.01; 
   ecm->Ea_R1 =  0.01;
   ecm->Ea_C1 = -0.01;

   /* Capacity and thermal */
   ecm->Q_Ah = Q_DESIGN; 	/* Ah */
   ecm->Cp = HEAT_CAPACITY;    	/* Thermal capacity J/°C */
   ecm->ht = HEAT_TRANS_COEF;  	/* Thermal resistance °C/W */

   /* Dynamic state defaults */
   ecm->soc = ecm->params->soc_tbl[SOC_GRIDS-1];
   ecm->V_oc = ecm->params->ocv_tbl[SOC_GRIDS-1];
   ecm->prev_V = ecm->V_oc;
   ecm->V_batt = ecm->prev_V;
   ecm->prev_I = 0.0;
   ecm->I = ecm->prev_I;
   ecm->T_C = T0_C;
   ecm->V_rc = 0.0;
   ecm->H = ecm->params->h_dsg_tbl[SOC_GRIDS-1];;

   /* R0, R1, C1 */
   ecm_lookup_r0(ecm, ecm->soc, &ecm->R0);
   ecm_lookup_r1(ecm, ecm->soc, &ecm->R1);
   ecm_lookup_c1(ecm, ecm->soc, &ecm->C1);


   ecm->chg_state = REST;

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
    flash_params_t *params = ecm->params;
    return tbl_interp(params->soc_tbl, params->ocv_tbl, SOC_GRIDS, soc, val);
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
    int rc = 0;

    *val = ecm->H;
    flash_params_t *params = ecm->params;

    if (ecm->chg_state==CHG)
       rc = tbl_interp(params->soc_tbl, params->h_chg_tbl, SOC_GRIDS, soc, val);
    else if (ecm->chg_state==DSG) 
       rc = tbl_interp(params->soc_tbl, params->h_dsg_tbl, SOC_GRIDS, soc, val);
    else
       *val = ecm->H;

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
    flash_params_t *params = ecm->params;
    return tbl_interp(params->soc_tbl, params->r0_tbl, SOC_GRIDS, soc, val);
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
    flash_params_t *params = ecm->params;
    return tbl_interp(params->soc_tbl, params->r1_tbl, SOC_GRIDS, soc, val);
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
    flash_params_t *params = ecm->params;
    return tbl_interp(params->soc_tbl, params->c1_tbl, SOC_GRIDS, soc, val);
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
    ecm->I = I;
    ecm->T_amb_C = T_amb_C;
   
    /* soc update */
    double Qmax = ecm->Q_Ah * 3600;
    ecm->soc -= (ecm->I*dt)/Qmax;

    /* temp update */
    double powerloss = ecm->I * ecm->I * ecm->R0;
    ecm->T_C += dt * (powerloss - ecm->ht * (ecm->T_C - ecm->T_amb_C)) / ecm->Cp;

    /* model param update */
    ecm_lookup_r0(ecm, util_clamp(ecm->soc, 0.0, 1.0), &R0);
    ecm->R0 = util_temp_adj(R0, ecm->Ea_R0, ecm->T_C, ecm->params->T_ref_C);

    ecm_lookup_r1(ecm, util_clamp(ecm->soc, 0.0, 1.0), &R1);
    ecm->R1 = util_temp_adj(R1, ecm->Ea_R1, ecm->T_C, ecm->params->T_ref_C); 

    ecm_lookup_c1(ecm, util_clamp(ecm->soc, 0.0, 1.0), &C1);
    ecm->C1 = util_temp_adj(C1, ecm->Ea_C1, ecm->T_C, ecm->params->T_ref_C);  


    /* update V_oc */
    ecm_lookup_ocv(ecm, ecm->soc, &ecm->V_oc);
    

    /* update V_rc */
    ecm->V_rc += dt * ( -ecm->V_rc / (ecm->R1 * ecm->C1) + ecm->I / ecm->C1 );


    /* Track charging state for hysteresis sign */
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


