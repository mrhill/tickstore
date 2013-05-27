#ifndef tsTICKRECEIVER_H
#define tsTICKRECEIVER_H

#include "tsSocket.h"
#include "tsTickQueue.h"

struct tsTickListener
{
    /** Process a received tick.
        @param pRawTick Serialized tick
        @param tickSize Byte size of serialized tick pointed to by \a pRawTick
    */
    virtual void ProcessTick(const char* pRawTick, bbUINT tickSize) = 0;
};

class tsTickReceiver
{
protected:
    tsSocket        mSocket;
    tsTickListener* mpListener;
    bbU64           mBytesReceived;
    bbU64           mTicksReceived;
    tsTickQueue     mTickQueue;

public:
    tsTickReceiver(tsTickFactory& tickFactory, tsTickListener* pListener, int socketFD, const char* pQueueName = NULL);

    inline tsSocket& socket() { return mSocket; }
    inline int socketFD() const { mSocket.fd(); }

    /** Receive data from socket
        @param timeout Timeout, see tsSocket::recv()
        @return Number of bytes received, -1 on error, 0 on connection closed
    */
    int receiveTicks(int timeout);
};

#endif

