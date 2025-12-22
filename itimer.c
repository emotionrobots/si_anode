/*!
 *======================================================================================================================
 *
 * @file	itimer.c
 *
 * @brief	itimer implementation
 *
 *======================================================================================================================
 */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

#include "itimer.h"


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void itimer_thread_fn(union sigval sv)
 *
 *  @brief	Runs in a thread created by the system for each expiration (or reused). 
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
void _itimer_thread_fn(union sigval sv)
{
    itimer_t *tm = (itimer_t *)sv.sival_ptr;
    if (!tm || !tm->isr) return;
    tm->isr(tm->usr_arg);
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int ms_to_itimerspec(long period_ms, struct itimerspec *out)
 *
 *  @brief	Convert millisconds to itimer spec
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
int _ms_to_itimerspec(long period_ms, struct itimerspec *out)
{
    if (!out || period_ms <= 0) return -EINVAL;

    memset(out, 0, sizeof(*out));

    time_t sec = period_ms / 1000;
    long nsec  = (period_ms % 1000) * 1000000L;

    out->it_interval.tv_sec  = sec;
    out->it_interval.tv_nsec = nsec;

    /* initial expiration = period (start after one period) */
    out->it_value.tv_sec  = sec;
    out->it_value.tv_nsec = nsec;

    return 0;
}



/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int itimer_create(itimer_t *tm, itimer_isr_fn isr, void *isr_arg)
 *
 *  @brief	Create an itimer with optional ISR and ISR argument
 *
 *  @note	Created itimer must be destroyed by calling itimer_destroy()
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
itimer_t *itimer_create(itimer_isr_fn isr, void *isr_arg)
{
    if (!isr) return NULL;

    itimer_t *tm = (itimer_t *)calloc(1, sizeof(itimer_t));
    if (!tm) return NULL;

    tm->isr = isr;
    tm->usr_arg = isr_arg;

    struct sigevent sev;
    memset(&sev, 0, sizeof(sev));
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = _itimer_thread_fn;
    sev.sigev_value.sival_ptr = tm; /* passes itimer_t* to handler thread */

    if (timer_create(CLOCK_MONOTONIC, &sev, &tm->tid) != 0) return NULL;

    pthread_mutex_init(&tm->mu, NULL);
    pthread_cond_init(&tm->cv, NULL);

    return tm;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int itimer_start(itimer_t *tm, long period_ms)
 *
 *  @brief	Start itimer
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int itimer_start(itimer_t *tm, long period_ms)
{
    if (!tm) return -EINVAL;

    struct itimerspec its;
    int rc = _ms_to_itimerspec(period_ms, &its);
    if (rc != 0) return rc;

    if (timer_settime(tm->tid, 0, &its, NULL) != 0) {
        return -errno;
    }
    return 0;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int itimer_stop(itimer_t *tm)
 *
 *  @brief	Stop itimer
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
int itimer_stop(itimer_t *tm)
{
    if (!tm) return -EINVAL;

    struct itimerspec its;
    memset(&its, 0, sizeof(its)); /* disarm: it_value = 0 */

    if (timer_settime(tm->tid, 0, &its, NULL) != 0) {
        return -errno;
    }
    return 0;
}

/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void itimer_destroy(itimer_t *tm)
 *  
 *  @brief	Destroy an itimer and freeing its memory
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void itimer_destroy(itimer_t *tm)
{
    if (!tm) return;
    timer_delete(tm->tid);
    free(tm);
}

