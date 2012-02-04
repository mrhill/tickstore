#include "tsTick.h"
#include <stdexcept>
#include <streamprintf.h>

std::string tsObjID::str() const
{
    return strprintf("ex=0x%08X,sym=0x%08X%08X", exchangeID(), (bbU32)(symbolID()>>32), (bbU32)symbolID());
}

std::string tsTick::str() const
{
    tsTime time(mTime);
    return strprintf("type=%d,ex=0x%08X,sym=0x%08X%08X,count=%u,time=%s", 
                     (int)mType,
                     mObjID.exchangeID(),
                     (bbU32)(mObjID.symbolID()>>32), (bbU32)mObjID.symbolID(),
                     mCount,
                     time.str());
}

int tsTick::serializeHead(char* pBuf, int tailSize) const
{
    bbU32 tmp;

    bbST16LE(pBuf, mType); pBuf+=2;
    tailSize+=tsTick::SERIALIZEDHEADSIZE; bbST16LE(pBuf, tailSize); pBuf+=2;

    bbST32LE(pBuf, mObjID.exchangeID()); pBuf+=4;
    tmp = (bbU32)mObjID.symbolID(); bbST32LE(pBuf, tmp); pBuf+=4;
    tmp = (bbU32)(mObjID.symbolID()>>32); bbST32LE(pBuf, tmp); pBuf+=4;

    bbST32LE(pBuf, mCount); pBuf+=4;
    tmp = (bbU32)mTime; bbST32LE(pBuf, tmp); pBuf+=4;
    tmp = (bbU32)(mTime>>32); bbST32LE(pBuf, tmp);

    return tsTick::SERIALIZEDHEADSIZE;
}

int tsTick::unserializeHead(const char* pBuf)
{
    mType = bbLD16LE(pBuf); pBuf+=4;
    mObjID.setExchangeID(bbLD32LE(pBuf)); pBuf+=4;
    mObjID.setSymbolID((bbU64)bbLD32LE(pBuf) | ((bbU64)(bbLD32LE(pBuf+4))<<32)); pBuf+=8;
    mCount = bbLD32LE(pBuf); pBuf+=4;
    mTime = (bbU64)bbLD32LE(pBuf) | ((bbU64)(bbLD32LE(pBuf+4))<<32);
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
    return tick.str() + ',' + strTail(&tick);
}

void tsTickDiag::serializeTail(char* pBuf) const
{
    bbU32 tmp;
    tmp = (bbU32)mSendTime; bbST32LE(pBuf, tmp); pBuf+=4;
    tmp = (bbU32)(mSendTime>>32); bbST32LE(pBuf, tmp);
}

void tsTickDiag::unserializeTail(const char* pBuf)
{
    mSendTime = (bbU64)bbLD32LE(pBuf) | ((bbU64)(bbLD32LE(pBuf+4))<<32);
}

std::string tsTickDiag::strTail() const
{
    tsTime time(mSendTime);
    return "sendtime=" + time.str();
}

