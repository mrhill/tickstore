#include "tsTickProcSchleuder.h"
#include "tsTickFinance.h"
#include <iostream>

tsTickProcSchleuder::tsTickProcSchleuder(tsTickFactory& tickFactory, tsStore& store, int socketFD, int procID, bbUINT group)
  : tsTickReceiver(tickFactory, socketFD, procID),
    mStore(store),
    mGroup(group & 255)
{
    mInFilter.AddFeed(400);
    mInFilter.AddFeed(29);
    mInFilter.AddFeed(0x42);
}

void tsTickProcSchleuder::Proc(const char* pRawTick, bbUINT tickSize)
{
    tsTickUnion tickUnion;
    int headSize = static_cast<tsTick&>(tickUnion).unserializeHead(pRawTick);

    if (static_cast<const tsTick&>(tickUnion).type() == tsTickType_Diag)
    {
        mStore.tickFactory().unserializeTail(pRawTick + headSize, &static_cast<tsTick&>(tickUnion));
        tsTickDiag& tickDiag = static_cast<tsTickDiag&>(static_cast<tsTick&>(tickUnion));
        tickDiag.setReceiveTime(tsTime::currentTimestamp());

        std::cout << mStore.tickFactory().str(tickDiag)
                  << strprintf(" latency send %d ms, receive %d ms",
                               (int)(((bbS64)tickDiag.sendTime() - (bbS64)tickDiag.time())/1000000),
                               (int)(((bbS64)tickDiag.receiveTime() - (bbS64)tickDiag.time())/1000000))
                  << std::endl;
    }
    else if (static_cast<const tsTick&>(tickUnion).type() == tsTickType_Auth)
    {
        mStore.tickFactory().unserializeTail(pRawTick + headSize, &static_cast<tsTick&>(tickUnion));
        tsTickAuth& tickAuth = static_cast<tsTickAuth&>(static_cast<tsTick&>(tickUnion));

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
    else if (mInFilter.isAllowed(static_cast<const tsTick&>(tickUnion).objID().feedID()))
    {
        mInFilter.mLastFeedIDCache = static_cast<const tsTick&>(tickUnion).objID().feedID();
        try
        {
            mStore.SaveTick(pRawTick, tickSize);
        }
        catch (tsStoreException& e)
        {
            std::cout << __FUNCTION__ << ": " << e.what();
        }
    }
}

tsMonSymbol* tsTickProcSchleuder::GetMon(const tsObjID& objID)
{
    tsMonSymbol* pMon;
    ObjIDMonMap::iterator it = mObjID2Mon.find(objID);

    if (it == mObjID2Mon.end())
        mObjID2Mon[objID] = pMon = new tsMonSymbol;
    else
        pMon = it->second;

    return pMon;
}

void tsTickProcSchleuder::Proc_(const tsTick& tick)
{
    bool setPrice  = false;
    bool setVolume = false;

    double price = 0;
    bbU64 volume = 0;

    switch(tick.type())
    {
    case tsTickType_Price:
        {
            const tsTickPrice& tickPrice = static_cast<const tsTickPrice&>(tick);
            price = tickPrice.price();
            setPrice = true;
        }
        break;

    case tsTickType_Volume:
        {
            const tsTickVolume& tickVolume = static_cast<const tsTickVolume&>(tick);
            volume = tickVolume.volume();
            setVolume = true;
        }
        break;

    case tsTickType_PriceVolume:
        {
            const tsTickPriceVolume& tickPriceVolume = static_cast<const tsTickPriceVolume&>(tick);
            price = tickPriceVolume.price();
            setPrice = true;
            volume = tickPriceVolume.volume();
            setVolume = true;
        }
        break;

    default:
        return;
    }

    tsMonSymbol* pMon = GetMon(tick.objID());
    if (!pMon)
        return;

    if (setPrice)
    {
        pMon->setPrice(price);

        if (!pMon->highValid() || price > pMon->high())
            pMon->setHigh(price);

        if (!pMon->lowValid() || price < pMon->low())
            pMon->setLow(price);

        if (!pMon->openValid())
            pMon->setOpen(price);
    }

    if (setVolume)
    {
        pMon->setVolume(volume);
        pMon->addTotalVolume(volume);
    }
}

