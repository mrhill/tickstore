#include "tsMutex.h"
#include "tsdef.h"
#include <iostream>
#include <string.h>
#include <stdexcept>

#ifndef _WIN32
#include <time.h>
#else
#include <windows.h>
#endif

void addToTimespec(struct timespec* pSpec, int timeoutMs)
{
    int timeout_sec = timeoutMs / 1000;
    int timeout_msec = timeoutMs - (timeout_sec * 1000);
    long tv_nsec_old = pSpec->tv_nsec;

    pSpec->tv_nsec += timeout_msec * 1000000;

    if (pSpec->tv_nsec < tv_nsec_old)
        timeout_sec++;

    pSpec->tv_sec += timeout_sec;
}

#ifdef _WIN32

#define PTW32_TIMESPEC_TO_FILETIME_OFFSET \
	  ( ((LONGLONG) 27111902 << 32) + (LONGLONG) 3577643008 )

static void
ptw32_filetime_to_timespec (const FILETIME * ft, struct timespec *ts)
     /*
      * -------------------------------------------------------------------
      * converts FILETIME (as set by GetSystemTimeAsFileTime), where the time is
      * expressed in 100 nanoseconds from Jan 1, 1601,
      * into struct timespec
      * where the time is expressed in seconds and nanoseconds from Jan 1, 1970.
      * -------------------------------------------------------------------
      */
{
  ts->tv_sec =
    (int) ((*(LONGLONG *) ft - PTW32_TIMESPEC_TO_FILETIME_OFFSET) / 10000000);
  ts->tv_nsec =
    (int) ((*(LONGLONG *) ft - PTW32_TIMESPEC_TO_FILETIME_OFFSET -
	    ((LONGLONG) ts->tv_sec * (LONGLONG) 10000000)) * 100);
}

static void getTimeoutSpec(struct timespec* pSpec, int timeout)
{
    FILETIME ft;
    SYSTEMTIME st;
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ft);
    ptw32_filetime_to_timespec(&ft, pSpec);
    addToTimespec(pSpec, timeout);
}

#endif

tsMutex::tsMutex()
{
    memset(&mMutex, 0, sizeof(mMutex));

    int err = pthread_mutex_init(&mMutex, NULL);
    if (err)
        throw std::runtime_error(strprintf("%s: Error %d on pthread_mutex_init\n", __FUNCTION__, err));
}

tsMutex::~tsMutex()
{
    int err = pthread_mutex_destroy(&mMutex);
    if (err)
        std::cerr << strprintf("%s: Error %d on pthread_mutex_destroy\n", __FUNCTION__, err);
}

bool tsMutex::lock(int timeout)
{
    struct timespec timeout_absolute;
    #ifdef _WIN32
    getTimeoutSpec(&timeout_absolute, timeout);
    #else
    clock_gettime(CLOCK_REALTIME, &timeout_absolute);
    addToTimespec(&timeout_absolute, timeout);
    #endif

    if (0 == pthread_mutex_timedlock(&mMutex, &timeout_absolute))
        return true;
    else
        return false;
}
