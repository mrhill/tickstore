#ifndef tsTICKPROC_H
#define tsTICKPROC_H

#include "tsTick.h"
#include "tsHeader.h"
#include "tsSocket.h"
#include "tsStore.h"
#include "tsThread.h"

/** Tick Processor.

    Data Storage, ticks

    per exID:
    <objID> | <tickstream>

    Data Storage, header

    per exID:
    <objID> | <header>

    
*/
class tsTickProc : tsThread
{
    tsStore& mStore;
    tsSocket mSocket;
    int      mProcID;
    bbU64    mBytesReceived;

    virtual void* run();
public:
    tsTickProc(tsStore& store, int socketFD, int procID);
    ~tsTickProc();

    bbERR Proc(const tsTick& tick);
};

#endif

