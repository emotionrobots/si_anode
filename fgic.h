/*!
 *=====================================================================================================================
 *
 *  @file		fgic.h
 *
 *  @brief		FGIC object header 
 *
 *=====================================================================================================================
 */
#ifndef __FGIC_H__
#define __FGIC_H__

#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>


#include "ecm.h"
#include "batt.h"
#include "ukf.h"
#include "flash_params.h"
#include "soc_ocv_lookup.h"


typedef struct {
   batt_t *batt;			// battery model
   ecm_t *ecm;				// ECM model pointer
   ukf_t *ukf;				// UKF object pointer 
   int period;				// FGIC run period in ms
   bool learning;			// vrc learning in progress 
   int buf_len;				// vrc buffer len 
   double vrc_x[VRC_BUF_SZ];		// V_rc buffer time 
   double vrc_y[VRC_BUF_SZ];		// V_rc buffer voltage 
   double I_meas;              		// measured I
   double V_meas;              		// measured V
   double T_meas;              		// measured T
   double V_chg;			// desired charging voltage
   double I_chg;			// desired charging current
   double V_noise;			// voltage measurement noise amplitude
   double I_noise;			// current measurement noise amplitude
   double T_noise;			// temperature measurement noise amplitude 
   double V_offset;			// voltage measurement offset
   double I_offset;			// current measurement offset 
   double T_offset;			// temperature measurement offset 
   double rest_time;			// rest time since I=0 
   double min_rest;			// minimum rest time in seconds 
   double dI_max;			// max dI 
   double dI_min;			// min dI 
   double dV_max;			// max dV 
   double dV_min;			// min dV
   double V_oc_est;			// V_oc estimate from CCV 
   double ah;                           // hysteresis decay rate 
   double I_sum;                        // REST current sum 
   int h_dir;                           // H table to update
}
fgic_t;

fgic_t *fgic_create(batt_t *batt, flash_params_t *p, double T0_C);
int fgic_get_cccv(fgic_t *fgic, double *cc, double *cv);
int fgic_update(fgic_t *fgic, double T_amb_C, double t, double dt);
void fgic_destroy(fgic_t *fgic);


#endif // __FGIC_H__
