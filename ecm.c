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
#include <math.h>
#include <string.h>
#include "ecm.h"


/*! 
 *--------------------------------------------------------------------------------------------------------------------- 
 *
 * @fn		double clamp(double x, double lo, double hi)
 *
 * @brief	Clamping function
 *
 *--------------------------------------------------------------------------------------------------------------------- 
 */
static 
double clamp(double x, double lo, double hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}


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
    soc = clamp(soc, grid[0], grid[n - 1]);
    int best = 0;
    double best_err = fabs(soc - grid[0]);
    for (int i = 1; i < n; ++i) 
    {
        double e = fabs(soc - grid[i]);
        if (e < best_err) {
            best_err = e;
            best = i;
        }
    }
    return best;
}


/*! 
 *--------------------------------------------------------------------------------------------------------------------- 
 *
 * @fn		double arrhenius_scale(double k_ref, double Ea, double T_C, double T_ref_C)
 *
 * @brief	Arrhenius scaling k(T) = k_ref * exp(-Ea * (1/T - 1/T_ref)).
 *              T, T_ref in °K 
 *
 * @note	if Ea > 0, if T > Tr, R < R_ref  
 *		if Ea < 0, if T > Tr, C > C_ref
 *
 *--------------------------------------------------------------------------------------------------------------------- 
 */
static 
double arrhenius_scale(double k_ref, double Ea, double T_C, double Tref_C)
{
    double T  = T_C     + 273.15;
    double Tr = Tref_C  + 273.15;

    if (T < 1.0)  T  = 1.0;
    if (Tr < 1.0) Tr = 1.0;

    double factor = exp( -Ea * (1.0 / T - 1.0 / Tr) );
    return k_ref * factor;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 * @fn          double arrhenius_inv_scale(double k_ref, double Ea, double T_C, double Tref_C)
 *
 * @brief       Arrhenius inverse scaling k_ref(T) = k_val * exp(Ea/R * (1/T - 1/T_ref)).
 *              T, T_ref in °C; internal conversion to Kelvin.
 *
 * @note        if Ea > 0, for T > Tr, R < R_ref
 *              if Ea < 0, for T > Tr, C > C_ref
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static
double arrhenius_inv_scale(double k_val, double Ea, double T_C, double Tref_C)
{
    double T  = T_C     + 273.15;
    double Tr = Tref_C  + 273.15;

    if (T < 1.0)  T  = 1.0;
    if (Tr < 1.0) Tr = 1.0;

    double factor = exp( Ea * (1.0 / T - 1.0 / Tr) );
    return k_val * factor;
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
   ecm->Ea_R0 =  10.0; 
   ecm->Ea_R1 =  10.0;
   ecm->Ea_C1 = -10.0;

   /* Capacity and thermal */
   ecm->Q_Ah = Q_DESIGN; /* Ah */
   ecm->C_th = 200.0;    /* Thermal capacity J/°C */
   ecm->R_th = 3.0;      /* Thermal resistance °C/W */

   /* Dynamic state defaults */
   ecm->soc = ecm->params->soc_tbl[SOC_GRIDS-1];
   ecm->v_oc = ecm->params->ocv_tbl[SOC_GRIDS-1];
   ecm->prev_V = ecm->v_oc;
   ecm->v_batt = ecm->prev_V;
   ecm->prev_I = 0.0;
   ecm->I = ecm->prev_I;
   ecm->T_C = T0_C;
   ecm->v_rc = 0.0;
   ecm->H = ecm->params->h_dsg_tbl[SOC_GRIDS-1];;

   ecm->I_quit = DEFAULT_I_QUIT;

   ecm->chg_state = REST;

   memset(ecm->vrc_buf, 0, VRC_BUF_SZ*sizeof(double));
   ecm->vrc_buf_len = 0;

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
 *  @fn		int ecm_lookup_h_chg(const ecm_t *ecm, double soc, double *val)
 *
 *  @brief	Read  H_chg table given SOC
 *
 *--------------------------------------------------------------------------------------------------------------------- 
 */
int ecm_lookup_h_chg(const ecm_t *ecm, double soc, double *val)
{
    flash_params_t *params = ecm->params;
    return tbl_interp(params->soc_tbl, params->h_chg_tbl, SOC_GRIDS, soc, val);
}


/*! 
 *--------------------------------------------------------------------------------------------------------------------- 
 *
 *  @fn		int ecm_lookup_h_dsg(const ecm_t *ecm, double soc, double *val)
 *  
 *  @brief	Return H_dsg given SOC
 *
 *--------------------------------------------------------------------------------------------------------------------- 
 */
int ecm_lookup_h_dsg(const ecm_t *ecm, double soc, double *val)
{
    flash_params_t *params = ecm->params;
    return tbl_interp(params->soc_tbl, params->h_dsg_tbl, SOC_GRIDS, soc, val);
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
 * @fn		int ecm_update(ecm_t *ecm, double I, double T, double t, double dt)
 * 
 * @brief	Compute the ECM dynamics one time step
 *
 * @param	ecm   : ECM instance
 * @param	I     : cell current [A], I > 0 = discharge, I < 0 = charge
 * @param	T     : cell temperature [°C]
 * @param	dt    : time step [s]
 *
 * @note	States updated: soc, v_rc, T, chg_state
 * 		Parameters R0/R1/C1 are read via lookup + Arrhenius scaling.
 *
 *--------------------------------------------------------------------------------------------------------------------- 
 */
int ecm_update(ecm_t *ecm, double I, double T, double t, double dt)
{
    int rc = 0;
    ecm->I = I;
    ecm->T_C = T;

    /* SOC update: I>0 discharge, I<0 charge */
    double dSOC = -(ecm->I * dt) / (ecm->Q_Ah * 3600.0);
    ecm->soc = clamp(ecm->soc + dSOC, 0.0, 1.0);

    /* model param update */
    ecm_lookup_r1(ecm, ecm->soc, &ecm->R1);
    ecm->R1 = arrhenius_scale(ecm->R1, ecm->Ea_R1, ecm->T_C, ecm->params->T_ref_C); 
    ecm_lookup_c1(ecm, ecm->soc, &ecm->C1);
    ecm->C1 = arrhenius_scale(ecm->C1, ecm->Ea_C1, ecm->T_C, ecm->params->T_ref_C);  
    ecm_lookup_r0(ecm, ecm->soc, &ecm->R0);
    ecm->R0 = arrhenius_scale(ecm->R0, ecm->Ea_R0, ecm->T_C, ecm->params->T_ref_C);

    /* update vrc and v_oc */
    ecm->v_rc += dt * ( -ecm->v_rc / (ecm->R1 * ecm->C1) + ecm->I / ecm->C1 );
    ecm_lookup_ocv(ecm, ecm->soc, &ecm->v_oc);
  

    /* Track charging state for hysteresis sign */
    ecm->prev_chg_state = ecm->chg_state;


    /* update H */
    if (ecm->I > ecm->I_quit)
    {
       ecm->chg_state = DSG; 		/* discharge */ 
       ecm_lookup_h_dsg(ecm, ecm->soc, &ecm->H);
    }
    else if (ecm->I < -ecm->I_quit)
    {
       ecm->chg_state = CHG; 		/* charge */
       ecm_lookup_h_chg(ecm, ecm->soc, &ecm->H);
    }
    else
    {
       ecm->chg_state = REST; 		/* rest, H no-change */
    }

    /* update v_batt */
    ecm->v_batt = (ecm->v_oc + ecm->H) - ecm->v_rc - ecm->I*ecm->R0;

    return rc;
}


