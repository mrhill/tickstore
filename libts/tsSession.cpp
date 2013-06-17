#include <iostream>
#include "tsSession.h"
#include "tsNode.h"

tsSession::tsSession(tsNode& node, int socketFD, int procID)
  : tsTickReceiver(this, socketFD),
    mNode(node),
    mTickSendCount(0),
    mSessionID(procID)
{
    printf("%s %d created\n", __FUNCTION__, mSessionID);
}

tsSession::~tsSession()
{
    printf("%s %d destroyed\n", __FUNCTION__, mSessionID);
}

void tsSession::ProcessTick(const char* pRawTick, bbUINT tickSize)
{
    tsTickUnion tickUnion;
    tsTick& tick = tickUnion;
    int headSize = tick.unserializeHead(pRawTick);

    switch (tick.type())
    {
    case tsTickType_Diag:
        {
        tsTickFactory::unserializeTail(pRawTick + headSize, &tick);
        tsTickDiag& tickDiag = static_cast<tsTickDiag&>(tick);
        tickDiag.setReceiveTime(tsTime::currentTimestamp());

        std::cout << tickDiag
                  << strprintf(" latency send %d ms, receive %d ms",
                               (int)(((bbS64)tickDiag.sendTime() - (bbS64)tickDiag.time())/1000000),
                               (int)(((bbS64)tickDiag.receiveTime() - (bbS64)tickDiag.time())/1000000))
                  << std::endl;
        }
        break;

    case tsTickType_Auth:
        {
        tsTickFactory::unserializeTail(pRawTick + headSize, &tick);
        tsTickAuth& tickAuth = static_cast<tsTickAuth&>(tick);

        std::cout << tickAuth << std::endl;

        mNode.Authenticate(mSessionID, tickAuth.mUID, tickAuth.mPwdHash);
        }
        break;

    case tsTickType_Subscribe:
        {
        tsTickFactory::unserializeTail(pRawTick + headSize, &tick);
        tsTickSubscribe& tickSubscribe = static_cast<tsTickSubscribe&>(tick);

        std::cout << tickSubscribe << std::endl;

        SubscribeFeed(tickSubscribe.feedID());
        }
        break;

    default:
        if (mInFilter.isAllowed(tick.objID().feedID()))
        {
            mInFilter.mLastFeedIDCache = tick.objID().feedID();
            mNode.ProcessTick(pRawTick, tickSize);
        }
        break;
    }
}

void tsSession::SendTick(tsTick& tick)
{
    char buf[tsTick::SERIALIZEDMAXSIZE];
    int tickSize = tsTickFactory::serializedSize(tick);
    tick.setCount(mTickSendCount);
    tsTickFactory::serialize(tick, buf);
    SendTick(buf, tickSize);
}

void tsSession::SendTick(const char* pRawTick, bbUINT tickSize)
{
    try
    {
        if (!mSocket.send(pRawTick, tickSize, 0))
            printf("%s %d: cannot send, discarding %d bytes\n", __FUNCTION__, mSessionID, tickSize);
        mTickSendCount++;
    }
    catch(tsSocketException& e)
    {
        printf("%s %d: discarding %d bytes, %s\n", __FUNCTION__, mSessionID, tickSize, e.what());
    }
}

void tsSession::SubscribeFeed(bbU64 feedID)
{
    mNode.SubscribeFeed(feedID, 0, this);
}

void tsSession::SetUser(const bbU8* pUser, bbUINT bufSize)
{
    if (!mUser.unserialize(pUser, bufSize))
        printf("%s %d: error unserializing tsUser (%d bytes)\n", __FUNCTION__, mSessionID, bufSize);

    if (mUser.perm() & tsUserPerm_TickToAll)
    {
        mInFilter.mAllowAll = 1;
    }
    else
    {
        mInFilter.mAllowAll = 0;
        const std::vector<bbU64> feeds = mUser.feeds();
        for(int i=0; i<feeds.size(); i++)
            mInFilter.AddFeed(feeds[i]);
    }
}

int tsSession::cmpSessionID(const void *p1, const void *p2)
{
    const tsSession* pSession = *(const tsSession**)p2;
    if (*(const int*)p1 < pSession->mSessionID)
        return -1;
    if (*(const int*)p1 > pSession->mSessionID)
        return 1;
    return 0;
}

