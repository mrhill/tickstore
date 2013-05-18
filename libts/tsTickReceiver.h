#ifndef tsTICKRECEIVER_H
#define tsTICKRECEIVER_H

#include "tsTick.h"
#include "tsHeader.h"
#include "tsSocket.h"
#include "tsStore.h"
#include "tsThread.h"
#include "tsTickQueue.h"

class tsTickReceiver : tsThread
{
    tsStore&    mStore;
    tsSocket    mSocket;
    int         mProcID;
    bbU64       mBytesReceived;
    bbU64       mTicksReceived;
    tsTickQueue mTickQueue;

    virtual void* run();
public:
    tsTickReceiver(tsTickFactory& tickFactory, tsStore& store, int socketFD, int procID);
    ~tsTickReceiver();

    /** Process a received tick.
        The default implementation deserializes the tick, and calls tsStore::SaveTick().
        @param pRawTick Serialized tick
        @param tickSize Byte size of serialized tick pointed to by \a pRawTick
    */
    virtual void Proc(const char* pRawTick, bbUINT tickSize);
};

#endif

