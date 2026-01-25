/*!
 *======================================================================================================================
 *
 * @file	ecm.h
 *
 * @brief	ECM module header
 *
 *======================================================================================================================
 */
#ifndef __ECM_H__
#define __ECM_H__

#include <stddef.h>
#include "globals.h"
#include "flash_params.h"


typedef struct 
{
   /* Flash tables */
   flash_params_t params;			/* pointer to flash */

   /* Model paramters */
   double R0, R1, C1;

   /* Model states */
   double V_batt;				/* V_batt */
   double V_rc;					/* VRC */ 
   double V_oc;					/* OCV */
   double soc;     				/* current SOC (0..1) */
   double H;       				/* OCV hysteresis */
   double T_C;       				/* core cell temperature [°C] */
   double T_amb_C;				/* ambient temp C */
   double I;					/* ECM current */
   double prev_I;				/* previous current */
   double prev_V_batt;				/* previous Vrc voltage */
   double prev_V_rc;				/* previous Vrc voltage */

   /* Arrhenius activation energies (J/mol) for R0, R1, C1 */
   double Ea_R0;
   double Ea_R1;
   double Ea_C1;

   /* Cell capacity (Ah) */
   double Q_Ah; 	

   /* Thermal */
   double Cp;    				/* thermal capacitance [J/°C] */
   double ht;    				/* thermal transfer [W/°C] */

   /* Hysteresis */
   double ah;					/* hysteresis decay rate */

   /* charging state 0 rest, -1 charge, +1 discharge */
   int chg_state;
   int prev_chg_state;

   /* Online parameter update bookkeeping */
   double I_quit;			       	/* Quit current */
  
   /* For R1/C1 LSQ on VRC decay to rest */
   double Vrc_buf[VRC_BUF_SZ];             	/* VRC data from entering rest */
   double Vrc_buf_len;                          /* buffer len */
} 
ecm_t;


int ecm_lookup_ocv(const ecm_t *ecm, double soc, double *val);
int ecm_lookup_h(const ecm_t *ecm, double soc, double *val);
int ecm_lookup_r0(const ecm_t *ecm, double soc, double *val);
int ecm_lookup_r1(const ecm_t *ecm, double soc, double *val);
int ecm_lookup_c1(const ecm_t *ecm, double soc, double *val);
int ecm_init(ecm_t *ecm, flash_params_t *p, double T0_C);
int ecm_update(ecm_t *ecm, double I, double T_amb, double t, double dt);
void ecm_cleanup(ecm_t *ecm);


#endif /* __ECM_H__ */

