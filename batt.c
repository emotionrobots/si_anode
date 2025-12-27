/*!
 *=====================================================================================================================
 *
 *  @file		batt.c
 *
 *  @brief		Battery object implementation 
 *
 *=====================================================================================================================
 */
#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include <time.h>
#include <errno.h>

#include "batt.h"


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		batt_t *batt_create(flash_params_t *p, double T0_C)
 *
 *  @brief	Battery init
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
batt_t *batt_create(flash_params_t *p, double T0_C)
{
   if (p == NULL) return -1;

   batt_t *batt = (batt_t *)calloc(1, sizeof(batt_t));
   if (batt != NULL)
   {
      batt->params = p;
      batt->ecm = (ecm_t *)malloc(sizeof(ecm_t));
      int rc = ecm_init(batt->ecm, p, T0_C);
      if (rc != 0) return NULL;
   }
   return batt;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int batt_update(batt_t *batt, double I, double T, double t, double dt)
 *
 *  @brief	Update battery model on time step
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int batt_update(batt_t *batt, double I, double T, double t, double dt)
{
   return ecm_update(batt->ecm, I, T, t, dt);
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void batt_destroy(batt_t *batt)
 *
 *  @brief	Clean up battery model
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void batt_destroy(batt_t *batt)
{
   if (batt != NULL)
   {
      if (batt->ecm != NULL) free(batt->ecm);
      free(batt);
   }
}


