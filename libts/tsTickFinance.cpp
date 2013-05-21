/*** This file is generated by makeTickFinance.py, do not edit ***/

#include "tsTickFinance.h"
#include <babel/StrBuf.h>

int tsTickFactoryFinance::serializedTailSize(const tsTick& tick) const
{
    switch(tick.mType)
    {
    case tsTickType_Price: return tsTickPrice::tailSize;
    case tsTickType_Volume: return tsTickVolume::tailSize;
    case tsTickType_PriceVolume: return tsTickPriceVolume::tailSize;
    case tsTickType_Bid: return tsTickBid::tailSize;
    case tsTickType_Ask: return tsTickAsk::tailSize;
    case tsTickType_BidAsk: return tsTickBidAsk::tailSize;
    case tsTickType_Recap: return tsTickRecap::tailSize;

    }

    return tsTickFactory::serializedTailSize(tick);
}

void tsTickFactoryFinance::serializeTail(const tsTick* pTick, char* pBuf) const
{
    switch(pTick->mType)
    {
    case tsTickType_Price: static_cast<const tsTickPrice*>(pTick)->serializeTail(pBuf); break;
    case tsTickType_Volume: static_cast<const tsTickVolume*>(pTick)->serializeTail(pBuf); break;
    case tsTickType_PriceVolume: static_cast<const tsTickPriceVolume*>(pTick)->serializeTail(pBuf); break;
    case tsTickType_Bid: static_cast<const tsTickBid*>(pTick)->serializeTail(pBuf); break;
    case tsTickType_Ask: static_cast<const tsTickAsk*>(pTick)->serializeTail(pBuf); break;
    case tsTickType_BidAsk: static_cast<const tsTickBidAsk*>(pTick)->serializeTail(pBuf); break;
    case tsTickType_Recap: static_cast<const tsTickRecap*>(pTick)->serializeTail(pBuf); break;

    default:
        tsTickFactory::serializeTail(pTick, pBuf);
        break;
    }
}

void tsTickFactoryFinance::unserializeTail(const char* pBuf, tsTick* pTick) const
{
    switch(pTick->mType)
    {
    case tsTickType_Price: static_cast<tsTickPrice*>(pTick)->unserializeTail(pBuf); break;
    case tsTickType_Volume: static_cast<tsTickVolume*>(pTick)->unserializeTail(pBuf); break;
    case tsTickType_PriceVolume: static_cast<tsTickPriceVolume*>(pTick)->unserializeTail(pBuf); break;
    case tsTickType_Bid: static_cast<tsTickBid*>(pTick)->unserializeTail(pBuf); break;
    case tsTickType_Ask: static_cast<tsTickAsk*>(pTick)->unserializeTail(pBuf); break;
    case tsTickType_BidAsk: static_cast<tsTickBidAsk*>(pTick)->unserializeTail(pBuf); break;
    case tsTickType_Recap: static_cast<tsTickRecap*>(pTick)->unserializeTail(pBuf); break;

    default:
        return tsTickFactory::unserializeTail(pBuf, pTick);
    }
}

std::string tsTickFactoryFinance::strTail(const tsTick* pTick) const
{
    switch(pTick->mType)
    {
    case tsTickType_Price: return static_cast<const tsTickPrice*>(pTick)->strTail();
    case tsTickType_Volume: return static_cast<const tsTickVolume*>(pTick)->strTail();
    case tsTickType_PriceVolume: return static_cast<const tsTickPriceVolume*>(pTick)->strTail();
    case tsTickType_Bid: return static_cast<const tsTickBid*>(pTick)->strTail();
    case tsTickType_Ask: return static_cast<const tsTickAsk*>(pTick)->strTail();
    case tsTickType_BidAsk: return static_cast<const tsTickBidAsk*>(pTick)->strTail();
    case tsTickType_Recap: return static_cast<const tsTickRecap*>(pTick)->strTail();

    default:
        return tsTickFactory::strTail(pTick);
    }
}


void tsTickPrice::serializeTail(char* pBuf) const
{
    union { bbU32 u32; float f32; bbU64 u64; double f64; };

    f64 = mPrice; bbST64LE(pBuf, u64); pBuf+=8;
    bbST32LE(pBuf, mOpt); pBuf+=4;
}

void tsTickPrice::unserializeTail(const char* pBuf)
{
    union { bbU32 u32; float f32; bbU64 u64; double f64; };

    u64 = bbLD64LE(pBuf); pBuf+=8; mPrice = f64;
    mOpt = bbLD32LE(pBuf); pBuf+=4;
}

std::string tsTickPrice::strTail() const
{
    bbStrBuf str;
    str.Printf(bbT(",price=%lg,opt=%u"), mPrice, mOpt);
    return std::string(str.GetPtr());
}


void tsTickVolume::serializeTail(char* pBuf) const
{
    bbST64LE(pBuf, mVolume); pBuf+=8;
    bbST32LE(pBuf, mOpt); pBuf+=4;
}

void tsTickVolume::unserializeTail(const char* pBuf)
{
    mVolume = bbLD64LE(pBuf); pBuf+=8;
    mOpt = bbLD32LE(pBuf); pBuf+=4;
}

std::string tsTickVolume::strTail() const
{
    bbStrBuf str;
    str.Printf(bbT(",volume=%"bbI64"u,opt=%u"), mVolume, mOpt);
    return std::string(str.GetPtr());
}


void tsTickPriceVolume::serializeTail(char* pBuf) const
{
    union { bbU32 u32; float f32; bbU64 u64; double f64; };

    f64 = mPrice; bbST64LE(pBuf, u64); pBuf+=8;
    bbST64LE(pBuf, mVolume); pBuf+=8;
    bbST32LE(pBuf, mOpt); pBuf+=4;
}

void tsTickPriceVolume::unserializeTail(const char* pBuf)
{
    union { bbU32 u32; float f32; bbU64 u64; double f64; };

    u64 = bbLD64LE(pBuf); pBuf+=8; mPrice = f64;
    mVolume = bbLD64LE(pBuf); pBuf+=8;
    mOpt = bbLD32LE(pBuf); pBuf+=4;
}

std::string tsTickPriceVolume::strTail() const
{
    bbStrBuf str;
    str.Printf(bbT(",price=%lg,volume=%"bbI64"u,opt=%u"), mPrice, mVolume, mOpt);
    return std::string(str.GetPtr());
}


void tsTickBid::serializeTail(char* pBuf) const
{
    union { bbU32 u32; float f32; bbU64 u64; double f64; };

    f64 = mPrice; bbST64LE(pBuf, u64); pBuf+=8;
    bbST64LE(pBuf, mVolume); pBuf+=8;
    bbST32LE(pBuf, mOpt); pBuf+=4;
}

void tsTickBid::unserializeTail(const char* pBuf)
{
    union { bbU32 u32; float f32; bbU64 u64; double f64; };

    u64 = bbLD64LE(pBuf); pBuf+=8; mPrice = f64;
    mVolume = bbLD64LE(pBuf); pBuf+=8;
    mOpt = bbLD32LE(pBuf); pBuf+=4;
}

std::string tsTickBid::strTail() const
{
    bbStrBuf str;
    str.Printf(bbT(",price=%lg,volume=%"bbI64"u,opt=%u"), mPrice, mVolume, mOpt);
    return std::string(str.GetPtr());
}


void tsTickAsk::serializeTail(char* pBuf) const
{
    union { bbU32 u32; float f32; bbU64 u64; double f64; };

    f64 = mPrice; bbST64LE(pBuf, u64); pBuf+=8;
    bbST64LE(pBuf, mVolume); pBuf+=8;
    bbST32LE(pBuf, mOpt); pBuf+=4;
}

void tsTickAsk::unserializeTail(const char* pBuf)
{
    union { bbU32 u32; float f32; bbU64 u64; double f64; };

    u64 = bbLD64LE(pBuf); pBuf+=8; mPrice = f64;
    mVolume = bbLD64LE(pBuf); pBuf+=8;
    mOpt = bbLD32LE(pBuf); pBuf+=4;
}

std::string tsTickAsk::strTail() const
{
    bbStrBuf str;
    str.Printf(bbT(",price=%lg,volume=%"bbI64"u,opt=%u"), mPrice, mVolume, mOpt);
    return std::string(str.GetPtr());
}


void tsTickBidAsk::serializeTail(char* pBuf) const
{
    union { bbU32 u32; float f32; bbU64 u64; double f64; };

    f64 = mPriceBid; bbST64LE(pBuf, u64); pBuf+=8;
    f64 = mPriceAsk; bbST64LE(pBuf, u64); pBuf+=8;
    bbST32LE(pBuf, mOpt); pBuf+=4;
}

void tsTickBidAsk::unserializeTail(const char* pBuf)
{
    union { bbU32 u32; float f32; bbU64 u64; double f64; };

    u64 = bbLD64LE(pBuf); pBuf+=8; mPriceBid = f64;
    u64 = bbLD64LE(pBuf); pBuf+=8; mPriceAsk = f64;
    mOpt = bbLD32LE(pBuf); pBuf+=4;
}

std::string tsTickBidAsk::strTail() const
{
    bbStrBuf str;
    str.Printf(bbT(",priceBid=%lg,priceAsk=%lg,opt=%u"), mPriceBid, mPriceAsk, mOpt);
    return std::string(str.GetPtr());
}


void tsTickRecap::serializeTail(char* pBuf) const
{
    union { bbU32 u32; float f32; bbU64 u64; double f64; };

    f64 = mOpen; bbST64LE(pBuf, u64); pBuf+=8;
    f64 = mHigh; bbST64LE(pBuf, u64); pBuf+=8;
    f64 = mLow; bbST64LE(pBuf, u64); pBuf+=8;
    f64 = mClose; bbST64LE(pBuf, u64); pBuf+=8;
    bbST64LE(pBuf, mVolume); pBuf+=8;
    bbST64LE(pBuf, mOpenInt); pBuf+=8;
    bbST32LE(pBuf, mOpt); pBuf+=4;
}

void tsTickRecap::unserializeTail(const char* pBuf)
{
    union { bbU32 u32; float f32; bbU64 u64; double f64; };

    u64 = bbLD64LE(pBuf); pBuf+=8; mOpen = f64;
    u64 = bbLD64LE(pBuf); pBuf+=8; mHigh = f64;
    u64 = bbLD64LE(pBuf); pBuf+=8; mLow = f64;
    u64 = bbLD64LE(pBuf); pBuf+=8; mClose = f64;
    mVolume = bbLD64LE(pBuf); pBuf+=8;
    mOpenInt = bbLD64LE(pBuf); pBuf+=8;
    mOpt = bbLD32LE(pBuf); pBuf+=4;
}

std::string tsTickRecap::strTail() const
{
    bbStrBuf str;
    str.Printf(bbT(",open=%lg,high=%lg,low=%lg,close=%lg,volume=%"bbI64"u,openInt=%"bbI64"u,opt=%u"), mOpen, mHigh, mLow, mClose, mVolume, mOpenInt, mOpt);
    return std::string(str.GetPtr());
}


