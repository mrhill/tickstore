#include "tsTickSender.h"
#include <stdexcept>
#include <iostream>
#include <stdexcept>
#include <sys/timeb.h>

void tsTickSender::tierDownConnection()
{
    printf(__FUNCTION__ ": tiering down connection\n");
    mSocket.close();
}

void* tsTickSender::run()
{
    int retryWait = 500;

    do
    {
        switch (mSocket.state())
        {
        case tsSocketState_Unconnected:
            try {
                printf(__FUNCTION__ ": connecting %s:%d...\n", mHostName.c_str(), mPort);
                mSocket.connect(mHostName.c_str(), mPort);
                retryWait = 500;
            } catch(tsSocketException& e) {
                printf(e.what());

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
                        bbST32LE(pTickSerialized+tsTick::SERIALIZEDHEADSIZE, (bbU32)t);
                        bbST32LE(pTickSerialized+tsTick::SERIALIZEDHEADSIZE+4, (bbU32)(t>>32));

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
                        tierDownConnection();
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

tsTickSender::tsTickSender(tsTickFactory& tickFactory, const char* pHostName, int port)
  : mTickCount(0), mLogLevel(2), mSocket(tsSocketType_TCP), mTickQueue(tickFactory), mHostName(pHostName), mPort(port)
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
    while (!mTickQueue.push(tick))
    {
        printf(__FUNCTION__ ": queue full, retry %d\n", ++retry);
        tsThread::msleep(100);
    }

    mTickCount++;
    mTickQueueSema.post();
}

void tsTickSender::sendDiagTick()
{
    tsTick diag(tsTickType_Diag);
    diag.setTime(tsTime::currentNs());
    sendUnprotected(diag);
}

void tsTickSender::send(tsTick& tick)
{
    mTickQueueWriteMutex.lock();

    //if (!(mTickCount & 63))
    //    sendDiagTick();
    sendUnprotected(tick);

    mTickQueueWriteMutex.unlock();
}

