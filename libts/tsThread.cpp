#include "tsThread.h"
#include "tsdef.h"
#include <string.h>
#include <iostream>
#include <stdexcept>

void* tsThread_run(void* arg)
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
        throw std::runtime_error(strprintf("%s: Error %d on pthread_create\n", __FUNCTION__, err));
    mRunning = 1;
}

void tsThread::join()
{
    if (mRunning)
    {
        int err = pthread_join(mThread, NULL);
        if (err)
            std::cerr << strprintf("%s: Error %d on pthread_join\n", __FUNCTION__, err);
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
