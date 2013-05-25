#include "tsSession.h"
#include "tsNode.h"
#include <iostream>

tsSession::tsSession(tsTickFactory& tickFactory, tsNode& node, tsStore& store, int fd, int procID)
  : tsTickReceiver(tickFactory, fd, procID), mNode(node), mStore(store)
{
    mInFilter.AddFeed(400);
    mInFilter.AddFeed(29);
    mInFilter.AddFeed(0x42);

    tsTickReceiver::start();
}

tsSession::~tsSession()
{
    cancel();
    join();
}

void* tsSession::run()
{
    void* ret = tsTickReceiver::run();
    mNode.DeactivateSession(this);
    return ret;
}

void tsSession::Proc(const char* pRawTick, bbUINT tickSize)
{
    tsTickUnion tickUnion;
    tsTick& tick = tickUnion;
    int headSize = tick.unserializeHead(pRawTick);

    switch (tick.type())
    {
    case tsTickType_Diag:
        {
        mStore.tickFactory().unserializeTail(pRawTick + headSize, &tick);
        tsTickDiag& tickDiag = static_cast<tsTickDiag&>(tick);
        tickDiag.setReceiveTime(tsTime::currentTimestamp());

        std::cout << mStore.tickFactory().str(tickDiag)
                  << strprintf(" latency send %d ms, receive %d ms",
                               (int)(((bbS64)tickDiag.sendTime() - (bbS64)tickDiag.time())/1000000),
                               (int)(((bbS64)tickDiag.receiveTime() - (bbS64)tickDiag.time())/1000000))
                  << std::endl;
        }
        break;

    case tsTickType_Auth:
        {
        mStore.tickFactory().unserializeTail(pRawTick + headSize, &tick);
        tsTickAuth& tickAuth = static_cast<tsTickAuth&>(tick);

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
        mStore.tickFactory().unserializeTail(pRawTick + headSize, &tick);
        SubscribeFeed(static_cast<tsTickSubscribe&>(tick).feedID());
        break;

    default:
        if (mInFilter.isAllowed(tick.objID().feedID()))
        {
            mInFilter.mLastFeedIDCache = tick.objID().feedID();
            try
            {
                mStore.SaveTick(pRawTick, tickSize);
            }
            catch (tsStoreException& e)
            {
                std::cout << __FUNCTION__ << ": " << e.what();
            }
        }
        break;
    }
}

void tsSession::SubscribeFeed(bbU64 feedID)
{
    mNode.SubscribeFeed(feedID, this);
}

