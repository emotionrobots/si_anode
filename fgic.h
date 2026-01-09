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


#include "ecm.h"
#include "batt.h"
#include "ukf.h"
#include "util.h"
#include "flash_params.h"
#include "soc_ocv_lookup.h"

typedef struct {
   double i;
   double v;
}
iv_t;


typedef struct {
   batt_t *batt;			// battery model
   flash_params_t *params;		// Pointer to flash params
   ecm_t *ecm;				// ECM model pointer
   ukf_t *ukf;				// UKF object pointer 
   int period;				// FGIC run period in ms
   bool learning;			// vrc learning in progress 
   int buf_len;				// vrc buffer len 
   iv_t vrc_buf[VRC_BUF_SZ];		// V_rc buffer to learn R1 C1
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
}
fgic_t;

fgic_t *fgic_create(batt_t *batt, flash_params_t *p, double T0_C);
int fgic_get_cccv(fgic_t *fgic, double *cc, double *cv);
int fgic_update(fgic_t *fgic, double T_amb_C, double t, double dt);
void fgic_destroy(fgic_t *fgic);


#endif // __FGIC_H__
