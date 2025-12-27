/*!
 *=====================================================================================================================
 *
 *  @file		util.c
 *
 *  @brief		Simulation utilities implementation 
 *
 *=====================================================================================================================
 */
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

