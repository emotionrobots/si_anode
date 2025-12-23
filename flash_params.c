/*!
 *===================================================================================================================== 
 *
 *  @file	flash_params.c
 *
 *  @brief	Flash params implementation 
 *
 *===================================================================================================================== 
 */
#include "flash_params.h"
#include "cell_chem.h"


flash_params_t g_flash_params = {
   .soc_tbl = { SOC_TBL },
   .ocv_tbl = { LFP_OCV_TBL }, 
   .r0_tbl  = { LFP_R0_TBL },
   .r1_tbl  = { LFP_R1_TBL },
   .c1_tbl  = { LFP_C1_TBL },

   .h_chg_tbl = {0},
   .h_dsg_tbl = {0},

   .design_capacity = 4.4,                     
   .v_end = 2.5,                              
   .v_full = 4.2,                           
   .T_ref_C = 25.0,

   .I_quit = DEFAULT_I_QUIT,
   .I_chg = DEFAULT_CC,
   .V_chg = DEFAULT_CV                             
};


