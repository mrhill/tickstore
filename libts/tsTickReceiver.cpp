#include "tsTickReceiver.h"
#include <iostream>

tsTickReceiver::tsTickReceiver(tsTickFactory& tickFactory, int socketFD, int procID)
  : mSocket(tsSocketType_TCP), mProcID(procID), mTickQueue(tickFactory)
{
    mSocket.setSocketDescriptor(socketFD);
}

tsTickReceiver::~tsTickReceiver()
{
}

void* tsTickReceiver::run()
{
    bbUINT qfullDelayMs = 32;
    bool connected = true;
    tsTickQueue::BufDesc qtail;

    printf("%s %d: connection from %s\n", __FUNCTION__, mProcID, mSocket.peerName().c_str());

    try
    {
        while (!testCancel() && (connected || !mTickQueue.empty()))
        {
            // - get pointers to free memory at queue tail
            if (!mTickQueue.backRaw(qtail))
            {
                printf("%s %d: receive queue full, wait %u ms\n", __FUNCTION__, mProcID, qfullDelayMs);
                tsThread::msleep(qfullDelayMs);
                if (qfullDelayMs < 4096)
                    qfullDelayMs <<= 1;
                continue;
            }
            qfullDelayMs = 32;

            int bytesReceived = mSocket.recv(qtail.pFirst, qtail.sizeFirst, 1000);
            if (bytesReceived <= 0)
            {
                if (bytesReceived == 0) // connection closed
                {
                    connected = false;
                }
                else
                {
                    printf("%s %d: mSocket.recv first timeout\n", __FUNCTION__, mProcID);
                    continue;
                }
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
                        printf("%s %d: receive queue deadlock detected, discarding %u bytes\n", __FUNCTION__, mProcID, mTickQueue.size());
                        mTickQueue.flush();
                    }
                    break; // not enough data in q to deserialize a tick
                }
                Proc(pRawTick, frontSize);
                mTickQueue.pop(frontSize);
                mTicksReceived++;

            } while(!mTickQueue.empty());
        }
    }
    catch(tsSocketException& e)
    {
        printf("%s %d: exception '%s'\n", __FUNCTION__, mProcID, e.what());
    }

    printf("%s %d: shutting down connection from %s (%d bytes left in q)\n", __FUNCTION__, mProcID, mSocket.peerName().c_str(), mTickQueue.size());

    return NULL;
}

