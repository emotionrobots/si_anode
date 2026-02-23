/*!
 *=====================================================================================================================
 *
 *  @file		cell_chem.h
 *
 *  @brief		Cell chemistry tables
 *
 *=====================================================================================================================
 */
#ifndef __CELL_CHEM_H__
#define __CELL_CHEM_H__

#include "globals.h"

/*!
 *---------------------------------------------------------------------------------------------------------------------
 * SOC table 
 *---------------------------------------------------------------------------------------------------------------------
 */
#define	SOC_TBL 		        0.00f, 0.05f, 0.10f, 0.15f, 0.20f,\
        			 	0.25f, 0.30f, 0.35f, 0.40f, 0.45f,\
        				0.50f, 0.55f, 0.60f, 0.65f, 0.70f,\
        				0.75f, 0.80f, 0.85f, 0.90f, 0.95f,\
        				1.00f


/*!
 *---------------------------------------------------------------------------------------------------------------------
 * LFP cell params
 *---------------------------------------------------------------------------------------------------------------------
 */
#define LFP_OCV_TBL			2.900f, 3.180f, 3.260f, 3.290f, 3.305f,\
    					3.315f, 3.320f, 3.325f, 3.330f, 3.332f,\
    					3.335f, 3.338f, 3.340f, 3.343f, 3.345f,\
    					3.350f, 3.360f, 3.380f, 3.410f, 3.480f,\
    					3.600f

#define LFP_R0_TBL 			0.0065f,0.0058f,0.0053f,0.0050f,0.0048f,\
					0.0046f,0.0045f,0.0044f,0.0043f,0.0043f,\
					0.0043f,0.0043f,0.0044f,0.0045f,0.0047f,\
					0.0049f,0.0052f,0.0056f,0.0061f,0.0068f,\
					0.0075f

#define LFP_R1_TBL 			0.0035f,0.0033f,0.0031f,0.0030f,0.0029f,\
					0.0028f,0.0027f,0.0026f,0.0026f,0.0025f,\
					0.0025f,0.0025f,0.0026f,0.0027f,0.0028f,\
					0.0029f,0.0031f,0.0033f,0.0036f,0.0039f,\
					0.0042f

#define LFP_C1_TBL 			1800,2000,2200,2400,2600,\
					2800,3000,3200,3400,3600,\
					3800,3600,3400,3200,3000,\
					2800,2600,2400,2200,2000,\
					1800

#define LFP_H_DSG_TBL 			DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,\
					DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,\
					DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,\
					DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,\
 					DEFAULT_H_DSG	

#define LFP_H_CHG_TBL 			DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,\
					DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,\
					DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,\
					DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,\
 					DEFAULT_H_CHG	
/*!
 *---------------------------------------------------------------------------------------------------------------------
 * NMC cell params
 *---------------------------------------------------------------------------------------------------------------------
 */
#define NMC_OCV_TBL 			3.000f, 3.320f, 3.500f, 3.600f, 3.650f,\
    					3.690f, 3.730f, 3.760f, 3.790f, 3.820f,\
    					3.850f, 3.875f, 3.900f, 3.930f, 3.960f,\
    					3.990f, 4.020f, 4.055f, 4.100f, 4.160f,\
    					4.200f

#define NMC_R0_TBL 			0.0045f,0.0040f,0.0037f,0.0035f,0.0033f,\
 					0.0032f,0.0031f,0.0030f,0.0030f,0.0029f,\
 					0.0029f,0.0029f,0.0030f,0.0031f,0.0033f,\
 					0.0035f,0.0038f,0.0042f,0.0048f,0.0055f,\
 					0.0062f

#define NMC_R1_TBL 			0.0028f,0.0026f,0.0025f,0.0024f,0.0023f,\
 					0.0022f,0.0021f,0.0020f,0.0020f,0.0019f,\
 					0.0019f,0.0020f,0.0021f,0.0022f,0.0023f,\
 					0.0025f,0.0027f,0.0030f,0.0034f,0.0039f,\
 					0.0044f

#define NMC_C1_TBL 			1500,1700,1900,2100,2300,\
 					2500,2700,2900,3100,3300,\
 					3500,3300,3100,2900,2700,\
 					2500,2300,2100,1900,1700,\
 					1500

#define NMC_H_DSG_TBL 			DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,\
					DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,\
					DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,\
					DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,\
 					DEFAULT_H_DSG	

#define NMC_H_CHG_TBL 			DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,\
					DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,\
					DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,\
					DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,\
 					DEFAULT_H_CHG	

#define NMC2_H_DSG_TBL 			DEFAULT2_H_DSG,DEFAULT2_H_DSG,DEFAULT2_H_DSG,DEFAULT2_H_DSG,DEFAULT2_H_DSG,\
					DEFAULT2_H_DSG,DEFAULT2_H_DSG,DEFAULT2_H_DSG,DEFAULT2_H_DSG,DEFAULT2_H_DSG,\
					DEFAULT2_H_DSG,DEFAULT2_H_DSG,DEFAULT2_H_DSG,DEFAULT2_H_DSG,DEFAULT2_H_DSG,\
					DEFAULT2_H_DSG,DEFAULT2_H_DSG,DEFAULT2_H_DSG,DEFAULT2_H_DSG,DEFAULT2_H_DSG,\
 					DEFAULT2_H_DSG	

#define NMC2_H_CHG_TBL 			DEFAULT2_H_CHG,DEFAULT2_H_CHG,DEFAULT2_H_CHG,DEFAULT2_H_CHG,DEFAULT2_H_CHG,\
					DEFAULT2_H_CHG,DEFAULT2_H_CHG,DEFAULT2_H_CHG,DEFAULT2_H_CHG,DEFAULT2_H_CHG,\
					DEFAULT2_H_CHG,DEFAULT2_H_CHG,DEFAULT2_H_CHG,DEFAULT2_H_CHG,DEFAULT2_H_CHG,\
					DEFAULT2_H_CHG,DEFAULT2_H_CHG,DEFAULT2_H_CHG,DEFAULT2_H_CHG,DEFAULT2_H_CHG,\
 					DEFAULT2_H_CHG	



/*!
 *---------------------------------------------------------------------------------------------------------------------
 * NCA cell params
 *---------------------------------------------------------------------------------------------------------------------
 */
#define NCA_OCV_TBL 			3.000f, 3.300f, 3.480f, 3.580f, 3.640f,\
    					3.690f, 3.730f, 3.760f, 3.790f, 3.820f,\
    					3.850f, 3.880f, 3.910f, 3.940f, 3.970f,\
    					4.000f, 4.030f, 4.070f, 4.115f, 4.170f,\
    					4.200f

#define NCA_R0_TBL 			0.0045f,0.0040f,0.0037f,0.0035f,0.0033f,\
        				0.0032f,0.0031f,0.0030f,0.0030f,0.0029f,\
        				0.0029f,0.0029f,0.0030f,0.0031f,0.0033f,\
        				0.0035f,0.0038f,0.0042f,0.0048f,0.0055f,\
        				0.0062f

#define NCA_R1_TBL 			0.0028f,0.0026f,0.0025f,0.0024f,0.0023f,\
        				0.0022f,0.0021f,0.0020f,0.0020f,0.0019f,\
        				0.0019f,0.0020f,0.0021f,0.0022f,0.0023f,\
        				0.0025f,0.0027f,0.0030f,0.0034f,0.0039f,\
        				0.0044f

#define NCA_C1_TBL 			1500,1700,1900,2100,2300,\
        				2500,2700,2900,3100,3300,\
        				3500,3300,3100,2900,2700,\
        				2500,2300,2100,1900,1700,\
        				1500

#define NCA_H_DSG_TBL 			DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,\
					DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,\
					DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,\
					DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,\
 					DEFAULT_H_DSG	

#define NCA_H_CHG_TBL 			DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,\
					DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,\
					DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,\
					DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,\
 					DEFAULT_H_CHG	

/*!
 *---------------------------------------------------------------------------------------------------------------------
 * LCO cell params
 *---------------------------------------------------------------------------------------------------------------------
 */
#define LCO_OCV_TBL 			3.000f, 3.360f, 3.540f, 3.640f, 3.700f,\
    					3.740f, 3.770f, 3.800f, 3.830f, 3.855f,\
    					3.880f, 3.905f, 3.930f, 3.955f, 3.985f,\
    					4.015f, 4.050f, 4.090f, 4.130f, 4.175f,\
    					4.200f

#define LCO_R0_TBL 			0.0045f,0.0040f,0.0037f,0.0035f,0.0033f,\
        				0.0032f,0.0031f,0.0030f,0.0030f,0.0029f,\
        				0.0029f,0.0029f,0.0030f,0.0031f,0.0033f,\
        				0.0035f,0.0038f,0.0042f,0.0048f,0.0055f,\
        				0.0062f

#define LCO_R1_TBL 			0.0032f,0.0030f,0.0028f,0.0027f,0.0026f,\
 					0.0025f,0.0024f,0.0023f,0.0023f,0.0022f,\
 					0.0022f,0.0023f,0.0024f,0.0025f,0.0026f,\
 					0.0028f,0.0030f,0.0033f,0.0037f,0.0042f,\
 					0.0048f

#define LCO_C1_TBL 			1500,1700,1900,2100,2300,\
        				2500,2700,2900,3100,3300,\
        				3500,3300,3100,2900,2700,\
        				2500,2300,2100,1900,1700,\
        				1500

#define LCO_H_DSG_TBL 			DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,\
					DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,\
					DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,\
					DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,\
 					DEFAULT_H_DSG	

#define LCO_H_CHG_TBL 			DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,\
					DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,\
					DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,\
					DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,\
 					DEFAULT_H_CHG	
/*!
 *---------------------------------------------------------------------------------------------------------------------
 * LMO cell params
 *---------------------------------------------------------------------------------------------------------------------
 */
#define LMO_OCV_TBL 			2.900f, 3.300f, 3.550f, 3.700f, 3.800f,\
    					3.860f, 3.900f, 3.940f, 3.970f, 3.990f,\
    					4.010f, 4.030f, 4.045f, 4.060f, 4.075f,\
    					4.090f, 4.105f, 4.120f, 4.135f, 4.145f,\
    					4.155f

#define LMO_R0_TBL 			0.0075f,0.0068f,0.0062f,0.0058f,0.0055f,\
 					0.0052f,0.0050f,0.0048f,0.0047f,0.0046f,\
 					0.0046f,0.0047f,0.0049f,0.0051f,0.0054f,\
 					0.0058f,0.0063f,0.0069f,0.0076f,0.0084f,\
 					0.0092f

#define LMO_R1_TBL 			0.0040f,0.0038f,0.0036f,0.0035f,0.0034f,\
 					0.0033f,0.0032f,0.0031f,0.0031f,0.0030f,\
 					0.0030f,0.0031f,0.0032f,0.0033f,0.0035f,\
 					0.0037f,0.0040f,0.0044f,0.0049f,0.0055f,\
 					0.0062f

#define LMO_C1_TBL 			1200,1400,1600,1800,2000,\
 					2200,2400,2600,2800,3000,\
 					3200,3000,2800,2600,2400,\
 					2200,2000,1800,1600,1400,\
 					1200

#define LMO_H_DSG_TBL 			DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,\
					DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,\
					DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,\
					DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,DEFAULT_H_DSG,\
 					DEFAULT_H_DSG	

#define LMO_H_CHG_TBL 			DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,\
					DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,\
					DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,\
					DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,DEFAULT_H_CHG,\
 					DEFAULT_H_CHG	

/*!
 *---------------------------------------------------------------------------------------------------------------------
 * LTO cell params
 *---------------------------------------------------------------------------------------------------------------------
 */
#define LTO_OCV_TBL 			2.070f, 2.100f, 2.120f, 2.140f, 2.160f,\
					2.170f, 2.180f, 2.190f, 2.200f, 2.220f,\
 					2.240f, 2.270f, 2.300f, 2.330f, 2.350f,\
				       	2.360f, 2.380f, 2.420f, 2.450f, 2.480f,\
					2.500f


/*--------------------------------------------------------------------------------------------------------------*/

#define ZERO_H_DSG_TBL                  0.0,0.0,0.0,0.0,0.0,\
					0.0,0.0,0.0,0.0,0.0,\
					0.0,0.0,0.0,0.0,0.0,\
					0.0,0.0,0.0,0.0,0.0,\
					0.0

#define ZERO_H_CHG_TBL                  0.0,0.0,0.0,0.0,0.0,\
					0.0,0.0,0.0,0.0,0.0,\
					0.0,0.0,0.0,0.0,0.0,\
					0.0,0.0,0.0,0.0,0.0,\
					0.0 

#endif // __CELL_CHEM_H__
