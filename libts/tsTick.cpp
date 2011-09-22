#include "tsTick.h"
#include <stdexcept>
#include <streamprintf.h>

std::string tsTick::str() const
{
    tsTime time(mTime);
    std::string str = strprintf("type=%d,ex=0x%08X,sym=0x%08X%08X,count=%u,time=%s", 
                                mType,
                                mObjID.exchangeID(),
                                (bbU32)(mObjID.symbolID()>>32), (bbU32)mObjID.symbolID(),
                                mCount,
                                time.str());
    switch(mType)
    {
    case tsTickType_None:        break;
    case tsTickType_Diag:        str += static_cast<const tsTickDiag*>(this)->str_tail(); break;
    case tsTickType_Price:       break;
    case tsTickType_Volume:      break;
    case tsTickType_PriceVolume: break;
    }

    return str;
}

void tsTick::serializeHead(char* pBuf, int tail) const
{
    bbU32 tmp;

    bbST16LE(pBuf, mType); pBuf+=2;
    tail+=tsTick::serializedHeadSize; bbST16LE(pBuf, tail); pBuf+=2;

    bbST32LE(pBuf, mObjID.exchangeID()); pBuf+=4;
    tmp = (bbU32)mObjID.symbolID(); bbST32LE(pBuf, tmp); pBuf+=4;
    tmp = (bbU32)(mObjID.symbolID()>>32); bbST32LE(pBuf, tmp); pBuf+=4;

    bbST32LE(pBuf, mCount); pBuf+=4;
    tmp = (bbU32)mTime; bbST32LE(pBuf, tmp); pBuf+=4;
    tmp = (bbU32)(mTime>>32); bbST32LE(pBuf, tmp);
}

void tsTick::unserializeHead(char* pBuf, tsTick* pTick)
{
    pTick->mType = (tsTickType)bbLD16LE(pBuf); pBuf+=4;
    pTick->mObjID.setExchangeID(bbLD32LE(pBuf)); pBuf+=4;
    pTick->mObjID.setSymbolID((bbU64)bbLD32LE(pBuf) | ((bbU64)(bbLD32LE(pBuf+4))<<32)); pBuf+=8;
    pTick->mCount = bbLD32LE(pBuf); pBuf+=4;
    pTick->mTime = (bbU64)bbLD32LE(pBuf) | ((bbU64)(bbLD32LE(pBuf+4))<<32);
}

int tsTick::serializedTailSize(const tsTick& tick)
{
    int tailSize;

    switch(tick.mType)
    {
    case tsTickType_None:        tailSize = 0; break;
    case tsTickType_Diag:        tailSize = tsTickDiag::tailSize; break;
    case tsTickType_Price:       tailSize = tsTickPrice::tailSize; break;
    case tsTickType_Volume:      tailSize = tsTickVolume::tailSize; break;
    case tsTickType_PriceVolume: tailSize = tsTickPriceVolume::tailSize; break;
    default:
        throw std::runtime_error(strprintf(__FUNCTION__ ": unknown TT %d", tick.mType));
    }

    return tailSize;
}

void tsTick::serialize(char* pBuf) const
{
    serializeHead(pBuf, serializedTailSize(*this));
    pBuf += tsTick::serializedHeadSize;

    switch(mType)
    {
    case tsTickType_Diag:        static_cast<const tsTickDiag*>(this)->serialize(pBuf); break;
    case tsTickType_Price:       static_cast<const tsTickPrice*>(this)->serialize(pBuf); break;
    case tsTickType_Volume:      static_cast<const tsTickVolume*>(this)->serialize(pBuf); break;
    case tsTickType_PriceVolume: static_cast<const tsTickPriceVolume*>(this)->serialize(pBuf); break;
    }
}

int tsTick::unserialize(char* pBuf, tsTick* pTick)
{
    int tickSize = bbLDA16LE(pBuf+2);

    if (pTick)
    {
        tsTick::unserializeHead(pBuf, pTick);
        pBuf += tsTick::serializedHeadSize;

        switch(pTick->mType)
        {
        case tsTickType_Diag:        static_cast<tsTickDiag*>(pTick)->unserialize(pBuf); break;
        case tsTickType_Price:       static_cast<tsTickPrice*>(pTick)->unserialize(pBuf); break;
        case tsTickType_Volume:      static_cast<tsTickVolume*>(pTick)->unserialize(pBuf); break;
        case tsTickType_PriceVolume: static_cast<tsTickPriceVolume*>(pTick)->unserialize(pBuf); break;
        }
    }

    return tickSize;
}

void tsTickDiag::serialize(char* pBuf) const
{
    bbU32 tmp;
    tmp = (bbU32)mSendTime; bbST32LE(pBuf, tmp); pBuf+=4;
    tmp = (bbU32)(mSendTime>>32); bbST32LE(pBuf, tmp);
}

void tsTickDiag::unserialize(const char* pBuf)
{
    mSendTime = (bbU64)bbLD32LE(pBuf) | ((bbU64)(bbLD32LE(pBuf+4))<<32);
}

std::string tsTickDiag::str_tail() const
{
    tsTime time(mSendTime);
    return ",sendtime="+time.str();
}
