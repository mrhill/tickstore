#ifndef tsTHREAD
#define tsTHREAD

#include <babel/defs.h>
#include <pthread.h>

class tsThread
{
    friend void* tsThread_run(void* arg);

    pthread_t     mThread;
    volatile bbU8 mCancel;
    bbU8          mRunning;

public:
    tsThread();
    ~tsThread();

    void start();
    void join();
    inline void cancel() {  mCancel = 1; }
    inline bool testCancel() { return mCancel != 0; }

    static void msleep(int ms)
    {
        #ifdef _WIN32
        Sleep(ms);
        #else
        usleep(ms*1000);
        #endif
    }

protected:
    virtual void* run() = 0;

};

#endif
