#include "tsSession.h"
#include "tsNode.h"
#include <iostream>

tsSession::tsSession(tsNode& node, tsStore& store, int socketFD, int procID)
  : tsTickReceiver(this, socketFD),
    mNode(node),
    mStore(store),
    mSessionID(procID)
{
    mInFilter.AddFeed(400);
    mInFilter.AddFeed(29);
    mInFilter.AddFeed(0x42);
}

tsSession::~tsSession()
{
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

        bbU64 allowedFeedID = mStore.Authenticate(tickAuth.mUID, tickAuth.mPwdHash);
        if (allowedFeedID == (bbU64)(bbS64)-1)
        {
            mInFilter.mAllowAll = 1;
        }
        else if (allowedFeedID)
        {
            mInFilter.mAllowAll = 0;
            mInFilter.AddFeed(allowedFeedID);
        }
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

void tsSession::SendTick(const char* pRawTick, bbUINT tickSize)
{
    try
    {
        if (!mSocket.send(pRawTick, tickSize, 0))
            printf("%s %d: cannot send, discarding %d bytes\n", __FUNCTION__, mSessionID, tickSize);
        else
            printf("%s %d: sent %d bytes\n", __FUNCTION__, mSessionID, tickSize);
    }
    catch(tsSocketException& e)
    {
        printf("%s: %s\n", __FUNCTION__, e.what());
    }
}

void tsSession::SubscribeFeed(bbU64 feedID)
{
    mNode.SubscribeFeed(feedID, this);
}

