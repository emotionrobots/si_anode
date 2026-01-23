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

#if 0
/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 * @fn          int util_nearest_soc_idx(const double *grid, int n, double soc)
 *
 * @brief       Find nearest SOC bin index.
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int util_nearest_soc_idx(const double *grid, int n, double soc)
{
    soc = util_clamp(soc, grid[0], grid[n - 1]);
    int best = 0;
    double best_err = fabs(soc - grid[0]);
    for (int i = 1; i < n; ++i)
    {
        double e = fabs(soc - grid[i]);
        if (e < best_err)
        {
            best_err = e;
            best = i;
        }
    }
    return best;
}
#endif


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int util_update_h_tbl(double *tbl, int n, double soc, double H)
 *
 *  @brief	Update H table entry indexed by SOC.  
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int util_update_h_tbl(double *h_tbl, double *soc_tbl, int n, double soc, double H)
{
   /* Find bookends indices where soc sits in between */
   int left = 0, right = 0;
   for (int i=0; i<n; i++)
   {
      if (soc_tbl[i] >= soc) 
      {
	 left = (i == 0) ? 0 : i-1; 
	 break;
      }
   }

   /* 
    * Update table. Closer soc is to the indexed entry, higher 
    * the influence H has on that entry's new value 
    */
   if (left == n-1)
   {
      h_tbl[left] = H;
   }
   else
   {  
      right = left+1;
      double denom = soc_tbl[right] - soc_tbl[left];
      double alpha = (soc-soc_tbl[left])/denom;
      h_tbl[left] = (1.0-alpha)*H + alpha*h_tbl[left];

      double beta = (soc_tbl[right]-soc)/denom;
      h_tbl[right] = (1.0-beta)*H + beta*h_tbl[right];
   }

   printf("soc=%lf, left=%d, right=%d\n", soc, left, right);

   return 0;
}


