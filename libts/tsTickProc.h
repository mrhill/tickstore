#ifndef tsTICKPROC_H
#define tsTICKPROC_H

#include "tsTick.h"
#include "tsHeader.h"
#include "tsSocket.h"
#include "tsStore.h"
#include "tsThread.h"
#include "tsTickQueue.h"

/** Tick Processor. */
class tsTickProc : tsThread
{
    tsStore&    mStore;
    tsSocket    mSocket;
    int         mProcID;
    bbU64       mBytesReceived;
    tsTickQueue mTickQueue;

    virtual void* run();
public:
    tsTickProc(tsTickFactory& tickFactory, tsStore& store, int socketFD, int procID);
    ~tsTickProc();

    virtual void Proc(const char* pRawTick);
    virtual void Proc(const tsTick& tick);
};

#endif

