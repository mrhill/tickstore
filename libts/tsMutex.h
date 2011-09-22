#ifndef tsMUTEX_H
#define tsMUTEX_H

#include <pthread.h>

class tsMutex
{
    pthread_mutex_t mMutex;
public:
    tsMutex();
    ~tsMutex();

    inline void lock()
    {
        pthread_mutex_lock(&mMutex);
    }

    /** Locks the mutex with timeout.
        @param timeout in ms
        @return true if mutex was locked, false if timed out
    */
    bool lock(int timeout);

    inline void unlock()
    {
        pthread_mutex_unlock(&mMutex);
    }
};

class tsMutexLocker
{
    tsMutex& mMutex;
public:
    tsMutexLocker(tsMutex& mutex) : mMutex(mutex) { mutex.lock(); }
    ~tsMutexLocker() { mMutex.unlock(); }
};

#endif
