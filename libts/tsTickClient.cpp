#include "tsTickClient.h"

tsTickClient::tsTickClient(const char* pQueueName, const char* pServerAddr, int port)
  : tsTickReceiver(this, -1),
    mServerAddr(pServerAddr),
    mServerPort(port),
    mTickCount(0)
{
    tsThread::start();
}

tsTickClient::~tsTickClient()
{
    tsThread::cancel();
    tsThread::join();
}

void* tsTickClient::run(void*)
{
    int retry = 0;
    int retryWait = 500;
    tsSocketSet socketSet;

    do
    {
        switch (mSocket.state())
        {
        case tsSocketState_Unconnected:
            try {
                printf("%s: connecting %s:%d, retry %d...\n", __FUNCTION__, mServerAddr.c_str(), mServerPort, retry++);
                mSocket.connect(mServerAddr.c_str(), mServerPort);
                retry = 0;
                retryWait = 500;
                printf("%s: connected to %s:%d...\n", __FUNCTION__, mServerAddr.c_str(), mServerPort);
            } catch(tsSocketException& e) {
                printf("%s", e.what());

                tsThread::msleep(retryWait);
                if (retryWait < 4000)
                    retryWait <<= 1;
            }
            break;

        case tsSocketState_Connected:
            socketSet.addRdFD(mSocket.fd());

            if (socketSet.select())
            {
                try
                {
                    if (socketSet.testRdFD(mSocket.fd()))
                    {
                        if (!receiveTicks(0))
                        {
                            printf("%s: closing TCP connection\n", __FUNCTION__);
                            mSocket.close();
                        }
                    }
                }
                catch(tsSocketException& e)
                {
                    printf("%s: %s\n", __FUNCTION__, e.what());
                }
            }
            break;

        default:
            tsThread::msleep(500);
            break;
        }

    } while (!testCancel());

    return NULL;
}

void tsTickClient::send(tsTick& tick)
{
    tick.setCount(mTickCount);

    bbUINT const tickSize = tsTickFactory::serializedSize(tick);
    char buf[tsTick::SERIALIZEDMAXSIZE];
    tsTickFactory::serialize(tick, buf);

    int retry = 0;
    int timeout = 100;
    for(;;)
    {
        if (mSocket.state() != tsSocketState_Connected)
        {
            printf("%s: retry %d, not connected\n", __FUNCTION__, retry);
        }
        else
        {
            try
            {
                if (mSocket.send(buf, tickSize, 0) > 0)
                    break;

                printf("%s: retry %d, cannot send tick\n", __FUNCTION__, retry);
            }
            catch(tsSocketException& e)
            {
                printf("%s: retry %d, %s\n", __FUNCTION__, retry, e.what());
            }
        }
        tsThread::msleep(timeout);
        if (timeout < 3200)
            timeout <<= 1;
        retry++;
    }

    mTickCount++;
}

void tsTickClient::ProcessTick(const char* pRawTick, bbUINT tickSize)
{
}

