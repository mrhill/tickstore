#include "tsTickProc.h"
#include "tsTickQueue.h"

tsTickProc::tsTickProc(tsStore& store, int socketFD, int procID) : mStore(store), mSocket(tsSocketType_TCP), mProcID(procID)
{
    mSocket.setSocketDescriptor(socketFD);
    start();
}

tsTickProc::~tsTickProc()
{
}

void* tsTickProc::run()
{
    bbUINT               qfullDelayMs = 32;
    bool                 connected = true;
    tsTickQueue          tickq;
    tsTickQueue::BufDesc qtail;
    tsTickUnion          tickUnion;

    printf(__FUNCTION__ " %d: connection from %s\n", mProcID, mSocket.peerName().c_str());

    try
    {
        while (!testCancel() && (connected || !tickq.empty()))
        {
            tickq.backRaw(qtail);

            if (qtail.sizeFirst == 0)
            {
                printf(__FUNCTION__ " %d: receive queue full, wait %u ms\n", mProcID, qfullDelayMs);
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
                    printf(__FUNCTION__ " %d: mSocket.recv first timeout\n", mProcID);
                    continue;
                }
            }

            if ((bytesReceived == qtail.sizeFirst) && qtail.sizeSecond) // more bytes may be pending on circular buffer wrap
            {
                int bytesReceivedTail = mSocket.recv(qtail.pSecond, qtail.sizeSecond, 0);
                if (bytesReceivedTail > 0) // 0 if connection closed or no byte available
                    bytesReceived += bytesReceivedTail;
            }

            tickq.pushRaw(bytesReceived); // commit received bytes into circular buffer
            mBytesReceived += bytesReceived;

            do
            {
                int frontSize = tickq.front(&tickUnion);
                if (frontSize <= 0)
                {
                    if (frontSize == -2)
                    {
                        printf(__FUNCTION__ " %d: receive queue deadlock detected, discarding %u bytes\n", mProcID, tickq.size());
                        tickq.flush();
                    }
                    break;
                }

                Proc(*tickUnion);
                tickq.pop(frontSize);

            } while(!tickq.empty());
        }
    }
    catch(tsSocketException& e)
    {
        printf(__FUNCTION__ " %d: exception '%s'\n", mProcID, e.what());
    }

    printf(__FUNCTION__ " %d: shutting down connection from %s (%d bytes left in q)\n", mProcID, mSocket.peerName().c_str(), tickq.size());

    return NULL;
}

bbERR tsTickProc::Proc(const tsTick& tick)
{
    //std::cout << tick.str() << std::endl;
    if (tick.type() == tsTickType_Diag)
    {
        const tsTickDiag& diag = static_cast<const tsTickDiag&>(tick);
        tsTime current = tsTime::current();

        std::cout << diag.str() << std::endl;
        printf("diag %d latency %d ms (%s - %s) %d ms\n", tick.count(), 
                                                    (int)(((bbS64)current.timestamp() - (bbS64)tick.time())/1000000), 
                                                    current.str().c_str(), tsTime(tick.time()).str().c_str(),
                                                    (int)(((bbS64)diag.sendTime() - (bbS64)tick.time())/1000000));
    }
    return bbEOK;
}
