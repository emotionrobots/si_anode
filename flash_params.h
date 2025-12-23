/*!
 *===================================================================================================================== 
 *
 *  @file	flash_params.h
 *
 *  @brief	Flash params definition
 *
 *===================================================================================================================== 
 */
#ifndef __FLASH_PARAMS_H__
#define __FLASH_PARAMS_H__

#include "globals.h"


/*!
 *---------------------------------------------------------------------------------------------------------------------
 * Flash parameters 
 *---------------------------------------------------------------------------------------------------------------------
 */
typedef struct {
   /* 
    * Battery settings 
    */
   double soc_tbl[SOC_GRIDS];                   /* SOC table; must spans [0,1] */
   double ocv_tbl[SOC_GRIDS];                   /* OCV table */
   double h_chg_tbl[SOC_GRIDS];                 /* CHG hysteresis table */
   double h_dsg_tbl[SOC_GRIDS];                 /* DSG hysteresis table */
   double r0_tbl[SOC_GRIDS];                    /* R0 table @ T_ref */
   double r1_tbl[SOC_GRIDS];                    /* R1 table @ T_ref */
   double c1_tbl[SOC_GRIDS];                    /* C1 table @ T_ref */

   double design_capacity;                      /* Battery design capacity */
   double v_end;                                /* End-of-discharge voltage */
   double v_full;                               /* full-charge voltage */
   double T_ref_C;                              /* Reference temperature in degC (tbl) */

   /* 
    * FGIC settings 
    */
   double I_quit;                               /* quit current */
   double I_chg;                                /* Default charging current */
   double V_chg;                                /* Default charging voltage */
}
flash_params_t;


#endif // __FLASH_PARAMS_H__
