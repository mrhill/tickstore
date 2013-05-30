#include "tsTickSender.h"
#include <stdexcept>
#include <iostream>
#include <stdexcept>
#include <sys/timeb.h>

void* tsTickSender::run()
{
    int retryWait = 500;

    do
    {
        switch (mSocket.state())
        {
        case tsSocketState_Unconnected:
            try {
                printf("%s: connecting %s:%d...\n", __FUNCTION__, mHostName.c_str(), mPort);
                mSocket.connect(mHostName.c_str(), mPort);
                retryWait = 500;
            } catch(tsSocketException& e) {
                printf("%s", e.what());

                tsThread::msleep(retryWait);
                if (retryWait < 4000)
                    retryWait <<= 1;
            }
            break;

        case tsSocketState_Connected:
            if (mTickQueueSema.wait(500))
            {
                char* pTickSerialized;
                int tickSize = mTickQueue.frontRaw(&pTickSerialized);

                if (tickSize > 0)
                {
                    if (bbLD16LE(pTickSerialized) == tsTickType_Diag)
                    {
                        bbU64 t = tsTime::currentNs();
                        bbST64LE(pTickSerialized+tsTick::SERIALIZEDHEADSIZE, t);

                        /*if (mLogLevel)
                        {
                            tsTickUnion tickUnion;
                            mTickQueue.front(&tickUnion);
                            const tsTick* pTick = tickUnion;
                            std::cout << pTick->str() << ' ' << ((tsTime::currentNs()-pTick->time())/1000000)  << std::endl;
                        }*/
                    }

                    try {
                        mSocket.send(pTickSerialized, tickSize);
                        mTickQueue.pop();
                    } catch(tsSocketException& e) {
                        std::cout << e.what();
                        printf("%s: tiering down connection\n", __FUNCTION__);
                        mSocket.close();
                        mTickQueueSema.post();
                    }
                }
            }
            break;

        default:
            tsThread::msleep(500);
            break;
        }

    } while (!testCancel() || (!mTickQueue.empty() && mSocket.state()==tsSocketState_Connected));

    return NULL;
}

tsTickSender::tsTickSender(const char* pQueueName, const char* pHostName, int port)
  : mTickCount(0), mLogLevel(2), mSocket(tsSocketType_TCP),
    mTickQueue(pQueueName, true), mHostName(pHostName), mPort(port)
{
    tsThread::start();
}

tsTickSender::~tsTickSender()
{
    tsThread::cancel();
    tsThread::join();
}

void tsTickSender::sendUnprotected(tsTick& tick)
{
    tick.setCount(mTickCount);

    int retry = 0;
    int timeout = 100;
    while (!mTickQueue.push(tick))
    {
        printf("%s: queue full, retry %d\n", __FUNCTION__, ++retry);
        tsThread::msleep(timeout);
        if (timeout < 3200)
            timeout <<= 1;
    }

    mTickCount++;
    mTickQueueSema.post();
}

void tsTickSender::sendDiagTick()
{
    tsTick diag(tsTickType_Diag);
    diag.setTime(tsTime::currentTimestamp());
    sendUnprotected(diag);
}

void tsTickSender::send(tsTick& tick)
{
    tsMutexLocker lock(mTickQueueWriteMutex);

    if (!(mTickCount & 63))
        sendDiagTick();
    sendUnprotected(tick);
}
