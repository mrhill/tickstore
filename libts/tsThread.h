#ifndef tsTHREAD
#define tsTHREAD

#include <babel/defs.h>
#include <pthread.h>
#ifndef _WIN32
#include <unistd.h>
#endif

class tsThread
{
    friend void* tsThread_run(void* arg);

    pthread_t     mThread;
    volatile int  mCancel;
protected:
    int           mRunning;

    virtual void* run() = 0;

public:
    tsThread();
    ~tsThread();

    void start();
    void join();
    inline void cancel() {  mCancel = 1; }
    inline bool testCancel() const { return mCancel != 0; }
    inline bool testRunning() const { return mRunning != 0; }

    static void msleep(int ms)
    {
        #ifdef _WIN32
        Sleep(ms);
        #else
        usleep(ms*1000);
        #endif
    }
};

#endif
