/*!
 *=====================================================================================================================
 *
 * @file	itimer.h
 *
 * @brief	iTimer header
 *
 *=====================================================================================================================
 */
#ifndef __ITIMER_H__
#define __ITIMER_H__

#include <time.h>
#include <pthread.h>


#ifdef __cplusplus
extern "C" {
#endif


/*!
 * itimer ISR function type
 */
typedef void (*itimer_isr_fn)(void *arg);


/*!
 * itimer context
 */
typedef struct {
    pthread_mutex_t mu;
    pthread_cond_t  cv;
    timer_t tid; 
    itimer_isr_fn isr;
    void *usr_arg;
}
itimer_t;



/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 * @fn			itimer_t *itimer_create(itimer_isr_fn isr, void *isr_arg);
 *
 * @brief		Create an itimer.
 *
 * @param out        	Returned timer handle
 * @param isr        	"Interrupt" handler callback
 * @param isr_arg    	Single argument passed to isr
 *
 * @return 	 	itimer_t pointer	
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
itimer_t *itimer_create(itimer_isr_fn isr, void *isr_arg);


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 * @fn			int itimer_start(itimer_t *t, long period_ms)
 *
 * @brief		Start (or restart) a periodic timer.
 *
 * @param t          	Timer handle
 * @param period_ms  	Period in milliseconds (must be > 0)
 *
 * @return 		0 on success, negative on error
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int itimer_start(itimer_t *t, long period_ms);


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 * @fn		int itimer_stop(itimer_t *t)
 *
 * @briief	Stop timer (disarm).
 *
 * @param t 	Timer handle
 *
 * @return 	0 on success, negative on error
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int itimer_stop(itimer_t *t);


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 * @fn		void itimer_destroy(itimer_t *t);
 *
 * @brief	Destroy timer and free resources.
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void itimer_destroy(itimer_t *t);



#ifdef __cplusplus
}
#endif

#endif // __ITIMER_H__

