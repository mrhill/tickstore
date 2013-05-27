#include "tsTickReceiver.h"
#include <iostream>

tsTickReceiver::tsTickReceiver(tsTickFactory& tickFactory, tsTickListener* pListener, int socketFD, const char* pQueueName)
  : mSocket(tsSocketType_TCP), mpListener(pListener), mTickQueue(tickFactory, pQueueName)
{
    mSocket.attachFD(socketFD);
}

int tsTickReceiver::receiveTicks(int timeout)
{
    tsTickQueue::BufDesc qtail;

    // - get pointers to free memory at queue tail
    if (!mTickQueue.backRaw(qtail))
    {
        printf("%s %s: receive queue full, flushing to prevent deadlock\n", __FUNCTION__, mTickQueue.name());
        mTickQueue.flush();
        bool check = mTickQueue.backRaw(qtail);
        bbASSERT(check);
    }

    int bytesReceived = mSocket.recv(qtail.pFirst, qtail.sizeFirst, timeout);
    if (bytesReceived < 0) // 0 if connection closed
    {
        printf("%s %s: mSocket.recv first timeout\n", __FUNCTION__, mTickQueue.name());
        return -1;
    }

    if ((bytesReceived == qtail.sizeFirst) && qtail.sizeSecond) // more bytes may be pending on circular buffer wrap
    {
        int bytesReceivedTail = mSocket.recv(qtail.pSecond, qtail.sizeSecond, 0);
        if (bytesReceivedTail > 0) // 0 if connection closed or no byte available
            bytesReceived += bytesReceivedTail;
    }

    mTickQueue.pushRaw(bytesReceived); // commit received bytes into circular buffer
    mBytesReceived += bytesReceived;

    do
    {
        char* pRawTick;
        int frontSize = mTickQueue.frontRaw(&pRawTick);
        if (frontSize <= 0)
        {
            if (frontSize == -2)
            {
                printf("%s %s: receive queue deadlock detected, discarding %u bytes\n", __FUNCTION__, mTickQueue.name(), mTickQueue.size());
                mTickQueue.flush();
            }
            break; // not enough data in q to deserialize a tick
        }
        mpListener->ProcessTick(pRawTick, frontSize);
        mTickQueue.pop(frontSize);
        mTicksReceived++;

    } while(!mTickQueue.empty());

    return bytesReceived;
}

