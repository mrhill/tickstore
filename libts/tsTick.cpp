#include "tsTick.h"
#include <stdexcept>

std::string tsObjID::str() const
{
    return strprintf("feed=0x%08X%08X,sym=0x%08X%08X", (bbU32)(feedID()>>32), (bbU32)feedID(), (bbU32)(symbolID()>>32), (bbU32)symbolID());
}

std::string tsTick::str() const
{
    tsTime time(mTime);
    return strprintf("tt=%d,feed=0x%08X%08X,sym=0x%08X%08X,count=%u,time=%s",
                     (int)mType,
                     (bbU32)(mObjID.feedID()>>32), (bbU32)mObjID.feedID(),
                     (bbU32)(mObjID.symbolID()>>32), (bbU32)mObjID.symbolID(),
                     mCount,
                     time.str().c_str());
}

int tsTick::serializeHead(char* pBuf, int tailSize) const
{
    // prefix
    bbST16LE(pBuf, mType); pBuf+=2;
    tailSize+=tsTick::SERIALIZEDHEADSIZE; bbST16LE(pBuf, tailSize); pBuf+=2;

    bbST64LE(pBuf, mObjID.feedID()); pBuf+=8;
    bbST64LE(pBuf, mObjID.symbolID()); pBuf+=8;
    bbST32LE(pBuf, mCount); pBuf+=4;
    bbST64LE(pBuf, mTime);

    return tsTick::SERIALIZEDHEADSIZE;
}

int tsTick::unserializeHead(const char* pBuf)
{
    // prefix
    mType = bbLD16LE(pBuf); pBuf+=4;

    mObjID.setFeedID(bbLD64LE(pBuf)); pBuf+=8;
    mObjID.setSymbolID(bbLD64LE(pBuf)); pBuf+=8;
    mCount = bbLD32LE(pBuf); pBuf+=4;
    mTime = bbLD64LE(pBuf);

    return tsTick::SERIALIZEDHEADSIZE;
}

int tsTickFactory::serializedTailSize(const tsTick& tick) const
{
    switch(tick.mType)
    {
    case tsTickType_Diag: return tsTickDiag::tailSize;
    default: return 0;
    }
}

void tsTickFactory::serializeTail(const tsTick* pTick, char* pBuf) const
{
    switch(pTick->mType)
    {
    case tsTickType_Diag: static_cast<const tsTickDiag*>(pTick)->serializeTail(pBuf); break;
    }
}

void tsTickFactory::unserializeTail(const char* pBuf, tsTick* pTick) const
{
    switch(pTick->mType)
    {
    case tsTickType_Diag: static_cast<tsTickDiag*>(pTick)->unserializeTail(pBuf); break;
    }
}

std::string tsTickFactory::strTail(const tsTick* pTick) const
{
    switch(pTick->mType)
    {
    case tsTickType_Diag: return static_cast<const tsTickDiag*>(pTick)->strTail();
    default: return "";
    }
}

void tsTickFactory::serialize(const tsTick& tick, char* pBuf) const
{
    pBuf += tick.serializeHead(pBuf, serializedTailSize(tick));
    serializeTail(&tick, pBuf);
}

int tsTickFactory::unserialize(const char* pBuf, tsTick* pTick) const
{
    int tickSize = bbLDA16LE(pBuf+2);

    if (pTick)
    {
        pBuf += pTick->unserializeHead(pBuf);
        unserializeTail(pBuf, pTick);
    }

    return tickSize;
}

std::string tsTickFactory::str(const tsTick& tick) const
{
    return tick.str() + strTail(&tick);
}

void tsTickDiag::serializeTail(char* pBuf) const
{
    bbST64LE(pBuf, mSendTime); pBuf+=8;
    bbST64LE(pBuf, mReceiveTime); pBuf+=8;
    bbST64LE(pBuf, mStoreTime); pBuf+=8;
}

void tsTickDiag::unserializeTail(const char* pBuf)
{
    mSendTime = bbLD64LE(pBuf); pBuf+=8;
    mReceiveTime = bbLD64LE(pBuf); pBuf+=8;
    mStoreTime = bbLD64LE(pBuf); pBuf+=8;
}

std::string tsTickDiag::strTail() const
{
    return ",sendtime=" + tsTime(mSendTime).str() + ",receivetime=" + tsTime(mReceiveTime).str();
}

