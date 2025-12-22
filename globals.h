/*!
 *=====================================================================================================================
 *
 *  @file		globals.h
 *
 *  @brief		Global defines 
 *
 *=====================================================================================================================
 */
#ifndef __GLOBALS_H__
#define __GLOBALS_H__


#define DT                      (0.25)          /* Second */
#define TEMP_0                  (25.0)          /* Degree C */
#define VRC_BUF_SZ              (64)            /* VRC buffer size */
#define SOC_GRIDS               (21)            /* SOC grip points */
#define MAX_RUN_TIME		(1000)		/* max simulation time in sec */
#define FGIC_RUN_PER_MS		(250)		/* FGIC run period (msec) */
#define DEFAULT_CC		(1)		/* Default charging current (A) */
#define DEFAULT_CV		(4.2)		/* Default charging voltage (V) */
#define DEFAULT_I_QUIT          (0.02)          /* Quit current (A) */

#define CHG			(-1)
#define REST			(0)
#define DSG			(1)


#endif // __GLOBALS_H__
