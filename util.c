/*!
 *=====================================================================================================================
 *
 *  @file		util.c
 *
 *  @brief		Simulation utilities implementation 
 *
 *=====================================================================================================================
 */
#include <math.h>
#include "globals.h"
#include "util.h"


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int sim_init(sim_t *sim)
 *
 *  @brief	Init simulation object
 *
 *  @return	0 if success; negative if error
 *
 *--------------------------------------------------------------------------------------------------------------------- 
 */
int util_msleep(long ms)
{
   struct timespec req, rem;

   req.tv_sec = ms / 1000L; 
   req.tv_nsec = (ms % 1000L) * 1000000L; 

   return nanosleep(&req, &rem);
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn         bool util_is_numeric(char *str)
 *
 *  @brief      Check if a string is numeric
 *
 *  @return     true or false
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
bool util_is_numeric(char *str)
{
  char *endptr;
  strtod(str, &endptr);
  return (str!=endptr);
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		double util_clamp(double x, double lo, double hi)
 *
 *  @brief	Clamp x between lo and hi
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
double util_clamp(double x, double lo, double hi)
{
   if (x > hi) return hi;
   if (x < lo) return lo;
   return x;
}

/*! 
 *--------------------------------------------------------------------------------------------------------------------- 
 *
 * @fn		double util_temp_adj(double k_ref, double Ea, double T_C, double Tref_C)
 *
 * @brief       Arrhenius scaling k(T) = k_ref * exp(-Ea/R * (1/T - 1/T_ref)).
 *              T, T_ref in °C; internal conversion to Kelvin.
 *
 * @note        if Ea > 0, for T > Tr, R < R_ref  
 *              if Ea < 0, for T > Tr, C > C_ref
 *
 *--------------------------------------------------------------------------------------------------------------------- 
 */
double util_temp_adj(double k_ref, double Ea, double T_C, double Tref_C)
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
 * @fn		double util_temp_unadj(double k_val, double Ea, double T_C, double Tref_C)
 *
 * @brief       Arrhenius inverse scaling k_ref(T) = k_val * exp(Ea/R * (1/T - 1/T_ref)).
 *              T, T_ref in °C; internal conversion to Kelvin.
 *
 * @note        if Ea > 0, for T > Tr, R < R_ref
 *              if Ea < 0, for T > Tr, C > C_ref
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
double util_temp_unadj(double k_val, double Ea, double T_C, double Tref_C)
{
    double T  = T_C     + 273.15;
    double Tr = Tref_C  + 273.15;

    if (T < 1.0)  T  = 1.0;
    if (Tr < 1.0) Tr = 1.0;

    double factor = exp( Ea * (1.0 / T - 1.0 / Tr) );
    return k_val * factor;
}

