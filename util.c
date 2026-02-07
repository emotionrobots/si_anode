/*!
 *=====================================================================================================================
 *
 *  @file		util.c
 *
 *  @brief		Simulation utilities implementation 
 *
 *=====================================================================================================================
 */
#include <string.h>
#include <math.h>
#include "globals.h"
#include "util.h"
#include "sim.h"


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
 *  @fn		int util_update_tbl(double *tbl, double *soc_tbl, int n, double soc, double val)
 *
 *  @brief	Update table entry indexed by SOC. Adjacent entries to left and right of SOC are 
 *              proportionally adjusted.
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int util_update_tbl(double *tbl, double *soc_tbl, int n, double soc, double val)
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
    * the influence val has on that entry's new value 
    */
   if (left == n-1)
   {
      tbl[left] = val;
   }
   else
   {  
      right = left+1;
      double denom = soc_tbl[right] - soc_tbl[left];
      double alpha = (soc-soc_tbl[left])/denom;
      tbl[left] = (1.0-alpha)*val + alpha*tbl[left];

      double beta = (soc_tbl[right]-soc)/denom;
      tbl[right] = (1.0-beta)*val + beta*tbl[right];
   }

   // printf("soc=%lf, left=%d, right=%d\n", soc, left, right);

   return 0;
}

/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn         int util_get_params_val(sim_t *sim, char *name, double *value)
 *
 *  @brief      Get parameter value 
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int util_get_params_val(sim_t *sim, char *name, double *value)
{
   int rc = -1;

   for (int i=0; i < sim->params_sz; i++)
   {
       if ( 0==strcmp(sim->params[i].name, name) && 0==strcmp(sim->params[i].type, "%lf"))
       {
          *value = *((double *)sim->params[i].value);
	  rc = 0;
          break;
       }
   }

   return rc;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn         char* util_get_params_type(sim_t *sim, char *name)
 *
 *  @brief      Get parameter type
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
char* util_get_params_type(sim_t *sim, char *name)
{
   char *ptype = NULL;

   for (int i=0; i < sim->params_sz; i++)
   {
       if (0==strcmp(sim->params[i].name, name))
       {
          ptype = sim->params[i].type;
          break;
       }
   }

   return ptype;
}

/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn         enum LOP util_strtolop(char *op)
 *
 *  @brief      Convert string to LOP
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
enum LOP util_strtolop(char *op)
{
   if (op == NULL) 
      return NOP;
   else if (0==strcmp(op, "=="))
      return EQ;
   else if (0==strcmp(op, ">"))
      return GT;
   else if (0==strcmp(op, ">="))
      return GTE;
   else if (0==strcmp(op, "<"))
      return LT;
   else if (0==strcmp(op, "<="))
      return LTE;
   else if (0==strcmp(op, "&&"))
      return AND;
   else if (0==strcmp(op, "||"))
      return OR;
   else
      return NOP;
}



/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn         char *util_loptostr(enum LOP lop)
 *
 *  @brief      Convert LOP to string
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
char *util_loptostr(enum LOP lop)
{
   if (lop == EQ)
      return "==";
   else if (lop == GT)
      return ">";
   else if (lop == GTE)
      return ">=";
   else if (lop == LT)
      return "<";
   else if (lop == LTE)
      return "<=";
   else if (lop == AND)
      return "&&";
   else if (lop == OR)
      return "||";
   else
      return NULL;
}


