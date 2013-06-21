#include "tsTickSender.h"
#include <stdexcept>
#include <iostream>
#include <stdexcept>
#include <sys/timeb.h>

tsTickSender::tsTickSender(const char* pQueueName, const char* pHostName, int port)
  : mTickCount(0),
    mSocket(tsSocketType_TCP),
    mHostName(pHostName),
    mPort(port),
    mLogFD(NULL),
    mUID(0)
{
    memset(mPwdHash, 0, sizeof(mPwdHash));

    if (pQueueName)
    {
        mQueueName = pQueueName;
        mLogFD = ::fopen((mQueueName + ".q.dat").c_str(), "ab");
    }
}

tsTickSender::~tsTickSender()
{
    if (mLogFD)
        ::fclose(mLogFD);
}

void tsTickSender::setLogin(bbU64 uid, const char* pPwdHash)
{
    mUID = uid;
    memcpy(mPwdHash, pPwdHash, sizeof(mPwdHash));
}

bool tsTickSender::authenticate()
{
    // to be called from connect()

    char buf[tsTick::SERIALIZEDMAXSIZE];

    printf("%s: authenticating with UID 0x%"bbI64"X:...\n", __FUNCTION__, mUID);

    tsTickAuth auth(mUID, mPwdHash);
    int tickSize = tsTickFactory::serialize(auth, buf);

    mSocket.send(buf, tickSize, -1);
    if (mLogFD)
        ::fwrite(buf, tickSize, 1, mLogFD);

    int expected = tsTick::SERIALIZEDHEADSIZE + tsTickAuthReply::TAILSIZE;
    int received = 0;
    do
    {
        int recvSize = mSocket.recv(buf+received, sizeof(buf)-received, 20000);
        if (!recvSize)
        {
            printf("%s: timeout waiting for reply\n", __FUNCTION__);
            return false; // timeout
        }
        received += recvSize;
    } while(received < expected);

    tsTickUnion tickUnion;
    tsTick& tick = tickUnion;
    tsTickFactory::unserialize(buf, &tick);

    if (tick.type() == tsTickType_AuthReply)
    {
        tsTickAuthReply& reply = static_cast<tsTickAuthReply&>(tick);
        printf("%s: result %s\n", __FUNCTION__, reply.UID()==mUID ? "OK" : "FAIL");
        return reply.UID()==mUID;
    }

    return false;
}

void tsTickSender::connect()
{
    int retryWait = 500;

    tsMutexLocker lock(mConnectMutex);

    for(;;)
    {
        switch (mSocket.state())
        {
        case tsSocketState_Unconnected:
            try {
                printf("%s: connecting %s:%d...\n", __FUNCTION__, mHostName.c_str(), mPort);
                mSocket.connect(mHostName.c_str(), mPort);
                retryWait = 500;

                if (!authenticate())
                    throw std::runtime_error("tsTickSender login failed");

            } catch(tsSocketException& e) {
                printf("%s: %s", __FUNCTION__, e.what());

                mSocket.close();

                tsThread::msleep(retryWait);
                if (retryWait < 4000)
                    retryWait <<= 1;
            }
            break;
        case tsSocketState_Connected:
            return;
        default:
            tsThread::msleep(500);
            break;
        }
    }
}

void tsTickSender::tierdown()
{
    tsMutexLocker lock(mConnectMutex);

    if (mSocket.state() != tsSocketState_Unconnected)
    {
        printf("%s: tiering down connection\n", __FUNCTION__);
        mSocket.close();
    }
}

void tsTickSender::send(tsTick& tick)
{
    tick.setCount(mTickCount);

    for(;;)
    {
        if (mSocket.state() != tsSocketState_Connected)
            connect();

        if (tick.type() == tsTickType_Diag)
            ((tsTickDiag&)tick).setSendTime(tsTime::currentNs());

        char buf[tsTick::SERIALIZEDMAXSIZE];
        int tickSize = tsTickFactory::serialize(tick, buf);

        try
        {
            mSocket.send(buf, tickSize, -1);
            mTickCount++; // xxx use atomic (not so important)

            if (mLogFD)
                ::fwrite(buf, tickSize, 1, mLogFD);

            if (!(mTickCount & 255))
            {
                tsTickDiag diag;
                diag.setTime(tsTime::currentNs());
                send(diag);
            }

            return;
        }
        catch(tsSocketException& e)
        {
            std::cout << e.what();
            tierdown();
        }
    }
}

