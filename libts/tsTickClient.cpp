#include "tsTickClient.h"
#include <iostream>

tsTickClient::tsTickClient(const char* pQueueName, const char* pServerAddr, int port)
  : mSocket(tsSocketType_TCP),
    mServerAddr(pServerAddr),
    mServerPort(port)
{
    tsThread::start();
}

tsTickClient::~tsTickClient()
{
    tsThread::cancel();
    tsThread::join();
}

void* tsTickClient::run()
{
    int retryWait = 500;
    do
    {
        switch (mSocket.state())
        {
        case tsSocketState_Unconnected:
            try {
                printf("%s: connecting %s:%d...\n", __FUNCTION__, mServerAddr.c_str(), mServerPort);
                mSocket.connect(mServerAddr.c_str(), mServerPort);
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

