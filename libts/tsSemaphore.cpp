#include "tsSemaphore.h"
#include <string.h>
#include <windows.h>
#include <streamprintf.h>

extern void getTimeoutSpec(struct timespec* pSpec, int timeout);

tsSemaphore::tsSemaphore(int initialCount)
{
    memset(&mSem, 0, sizeof(mSem));

    int err = sem_init(&mSem, 0, initialCount); 
    if (err)
        throw std::runtime_error(strprintf(__FUNCTION__ ": Error %d on sem_init\n", err));
}

tsSemaphore::~tsSemaphore()
{
    sem_destroy(&mSem);
}

bool tsSemaphore::wait(int timeout)
{
    struct timespec timeout_absolute;
    getTimeoutSpec(&timeout_absolute, timeout);

    if (0 == sem_timedwait(&mSem, &timeout_absolute))
        return true;
    else
        return false;
}
