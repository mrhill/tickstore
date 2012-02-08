#ifndef tsSEMAPHORE
#define tsSEMAPHORE

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
class tsSemaphore
{
    sem_t mSem;
public:
    tsSemaphore(int initialCount = 0);
    ~tsSemaphore();

    bool wait(int timeout);

    inline void post()
    {
        sem_post(&mSem);
    }
};

#endif
