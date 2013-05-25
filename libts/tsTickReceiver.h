#ifndef tsTICKRECEIVER_H
#define tsTICKRECEIVER_H

#include "tsSocket.h"
#include "tsTickQueue.h"

class tsTickReceiver
{
protected:
    bbU64       mBytesReceived;
    bbU64       mTicksReceived;
    tsTickQueue mTickQueue;

public:
    tsTickReceiver(tsTickFactory& tickFactory, const char* pQueueName = NULL);

    /** Receive data from socket
        @param socket Socket
        @param timeout Timeout, see tsSocket::recv()
    */
    int receive(tsSocket& socket, int timeout);

    /** Process a received tick.
        @param pRawTick Serialized tick
        @param tickSize Byte size of serialized tick pointed to by \a pRawTick
    */
    virtual void Proc(const char* pRawTick, bbUINT tickSize) = 0;
};

#endif

