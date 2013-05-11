#include "tsdef.h"
#include "tsSemaphore.h"
#include <string.h>
#ifdef _WIN32
#include <windows.h>
extern void getTimeoutSpec(struct timespec* pSpec, int timeout);
#else
#include <time.h>
#endif
#include <stdexcept>

extern void addToTimespec(struct timespec* pSpec, int timeoutMs);

tsSemaphore::tsSemaphore(int initialCount)
{
    memset(&mSem, 0, sizeof(mSem));

    int err = sem_init(&mSem, 0, initialCount); 
    if (err)
        throw std::runtime_error(strprintf("%s: Error %d on sem_init\n", __FUNCTION__, err));
}

tsSemaphore::~tsSemaphore()
{
    sem_destroy(&mSem);
}

bool tsSemaphore::wait(int timeout)
{
    struct timespec timeout_absolute;

    #ifdef _WIN32
    getTimeoutSpec(&timeout_absolute, timeout);
    #else
    clock_gettime(CLOCK_REALTIME, &timeout_absolute);
    addToTimespec(&timeout_absolute, timeout);
    #endif

    if (0 == sem_timedwait(&mSem, &timeout_absolute))
        return true;
    else
        return false;
}
