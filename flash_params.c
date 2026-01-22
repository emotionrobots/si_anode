/*!
 *===================================================================================================================== 
 *
 *  @file	flash_params.c
 *
 *  @brief	Flash params implementation 
 *
 *===================================================================================================================== 
 */
#include "globals.h"
#include "flash_params.h"
#include "cell_chem.h"


flash_params_t g_batt_flash_params = {
   .soc_tbl = { SOC_TBL },
   .ocv_tbl = { OCV_TBL }, 
   .r0_tbl  = { R0_TBL },
   .r1_tbl  = { R1_TBL },
   .c1_tbl  = { C1_TBL },

   .h_chg_tbl = { BATT_H_CHG_TBL },
   .h_dsg_tbl = { BATT_H_DSG_TBL },

   .design_capacity = 4.4,                     
   .v_end = 2.5,                              
   .T_ref_C = TEMP_0,

   .I_quit = DEFAULT_I_QUIT,
   .I_chg = DEFAULT_CC,
   .V_chg = DEFAULT_CV                             
};


flash_params_t g_fgic_flash_params = {
   .soc_tbl = { SOC_TBL },
   .ocv_tbl = { OCV_TBL },
   .r0_tbl  = { R0_TBL },
   .r1_tbl  = { R1_TBL },
   .c1_tbl  = { C1_TBL },

   .h_chg_tbl = { FGIC_H_CHG_TBL },
   .h_dsg_tbl = { FGIC_H_DSG_TBL },

   .design_capacity = 4.4,
   .v_end = 2.5,
   .T_ref_C = TEMP_0,

   .I_quit = DEFAULT_I_QUIT,
   .I_chg = DEFAULT_CC,
   .V_chg = DEFAULT_CV
};

