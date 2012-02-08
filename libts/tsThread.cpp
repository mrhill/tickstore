#include "tsThread.h"
#include "tsdef.h"
#include <string.h>
#include <iostream>

static void* tsThread_run(void* arg)
{
    return ((tsThread*)arg)->run();
}

tsThread::tsThread() : mCancel(0), mRunning(0)
{
    memset(&mThread, 0, sizeof(mThread));
}

void tsThread::start()
{
    int err = pthread_create(&mThread, NULL, tsThread_run, this);
    if (err)
        throw std::runtime_error(strprintf(__FUNCTION__ ": Error %d on pthread_create\n", err));
    mRunning = 1;
}

void tsThread::join()
{
    if (mRunning)
    {
        int err = pthread_join(mThread, NULL);
        if (err)
            std::cerr << strprintf(__FUNCTION__ ": Error %d on pthread_join\n", err);
        mRunning = 0;
    }
}

tsThread::~tsThread()
{
    if (mRunning)
    {
        cancel();
        join();
    }
}
