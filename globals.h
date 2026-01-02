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


#define LOCK(x)			pthread_mutex_lock(x)
#define UNLOCK(x)		pthread_mutex_unlock(x)

#define M_PI 			(3.14159265358979323846)

#define SYS_LOAD_CC             (0)
#define SYS_LOAD_PULSE          (1)
#define SYS_LOAD_OSC            (2)

#define DT                      (0.25)          /* Second */
#define TEMP_0                  (25.0)          /* Degree C */
#define VRC_BUF_SZ              (64)            /* VRC buffer size */
#define SOC_GRIDS               (21)            /* SOC grip points */
#define MAX_RUN_TIME		(1000000)	/* max simulation time in sec */
#define FGIC_PERIOD_MS		(250)		/* FGIC run period (msec) */
#define DEFAULT_CC		(1)		/* Default charging current (A) */
#define DEFAULT_CV		(4.2)		/* Default charging voltage (V) */
#define DEFAULT_I_QUIT          (0.02)          /* Quit current (A) */
#define MAX_LINE_SZ		(200)		/* max command line size */
#define MAX_TOKENS		(10)		/* max number of command line tokens */
#define MAX_PARAMS		(100)		/* max number of string-enabled parameters */
#define FN_LEN			(80)		/* logfile name length */
#define MAX_PLOT_PTS		(200000)	/* max number of string-enabled parameters */
#define DEFAULT_H_CHG		(0.01)		/* default OCV chg hysteresis */
#define DEFAULT_H_DSG		(0.01)		/* default OCV dsg hysteresis */
#define OCV_TBL			NMC_OCV_TBL	/* OCV table */
#define R0_TBL			NMC_R0_TBL	/* R0 table */
#define R1_TBL			NMC_R1_TBL	/* R1 table */
#define C1_TBL			NMC_C1_TBL	/* C1 table */
#define Q_DESIGN		(4.0)		/* 4 Ah */
#define DEFAULT_SYS_LOAD	SYS_LOAD_CC	/* default system load type */


#define CHG			(-1)
#define REST			(0)
#define DSG			(1)


/*!
 *---------------------------------------------------------------------------------------------------------------------
 * string-enabled parameters
 *---------------------------------------------------------------------------------------------------------------------
 */
typedef struct
{
   char *name;
   char *type;
   void *value;
}
params_t;


#endif // __GLOBALS_H__
