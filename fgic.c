/*!
 *=====================================================================================================================
 *
 *  @file		fgic.c
 *
 *  @brief		FGIC object implementation 
 *
 *=====================================================================================================================
 */
#include "fgic.h"

extern flash_params_t g_flash_params;

/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int fgic_init(fgic_t *fgic, flash_param_t *p, double T0_C)
 *
 *  @brief	Init FGIC
 *
 *  @return	0 if success; negative otherwise
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int fgic_init(fgic_t *fgic, flash_params_t *p, double T0_C)
{
   if (fgic == NULL) return -1;

   fgic->ecm = (ecm_t *)malloc(sizeof(ecm_t));
   int rc = ecm_init(fgic->ecm, &g_flash_params, T0_C);

   return rc;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int fgic_get_cccv(fgic_t *fgic, double *cc, double *cv)
 *
 *  @brief	Read back CC and CV
 *
 *  @return	0 if success; negative otherwise
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int fgic_get_cccv(fgic_t *fgic, double *cc, double *cv)
{
   if (fgic == NULL || cc == NULL || cv == NULL) return -1;

   *cc = fgic->I_chg;
   *cv = fgic->V_chg;

   return 0;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int fgic_update(fgic_t *fgic, double t, double dt)
 *
 *  @brief	Update FGIC one time step
 *
 *  @return	0 if success; negative otherwise
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int fgic_update(fgic_t *fgic, double I, double T, double t, double dt)
{
   if (fgic == NULL || fgic->ecm == NULL) return -1;

   int rc = ecm_update(fgic->ecm, I, T, t, dt);

   return rc;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int fgic_cleanup(fgic_t *fgic)
 *
 *  @brief	Clean-up FGIC
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void fgic_cleanup(fgic_t *fgic)
{
   if (fgic != NULL && fgic->ecm != NULL) 
   {
      ecm_cleanup(fgic->ecm);
      free(fgic->ecm);
   }
}

