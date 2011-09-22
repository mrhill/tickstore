#ifndef tsTICK_H
#define tsTICK_H

#include "tsObjID.h"
#include "tsTime.h"
#include <string>
#pragma warning(disable: 4355)

enum tsTickType
{
    tsTickType_None = 0,
    tsTickType_Diag,
    tsTickType_Price,
    tsTickType_Volume,
    tsTickType_PriceVolume,
    tsTickType_Bid,
    tsTickType_Ask,
    tsTickType_BidAsk,
    tsTickTypeCount
};

class tsTickQueue;

struct tsTick
{
    tsObjID     mObjID;
    tsTickType  mType;
    bbU32       mCount;
    bbU64       mTime;

    static const int serializedPrefix = 2+2;
    static const int serializedHeadSize = serializedPrefix+2+2+(4+8)+4+8;
    static const int serializedSizeMax = serializedHeadSize + 128;

    tsTick(tsTickType type = tsTickType_None) : mType(type), mCount(0), mTime(0) {}
    tsTick(const tsObjID& objID, tsTickType type = tsTickType_None) : mObjID(objID), mType(tsTickType_None), mCount(0), mTime(0) {}

    inline tsTickType type() const { return mType; }

    inline const tsObjID& objID() const { return mObjID; }
    inline void setObjID(const tsObjID& objID) { mObjID=objID; }

    inline bbU32 count() const { return mCount; }
    inline void setCount(bbU32 count) { mCount = count; }

    inline bbU64 time() const { return mTime; }
    inline void setTime(bbU64 timestamp) { mTime = timestamp; }
    inline void setTime(const tsTime& time) { mTime = time.timestamp(); }

    std::string str() const;

    int serializedSize() const { return tsTick::serializedTailSize(*this) + tsTick::serializedHeadSize; }
    void serialize(char* pBuf) const;
    static int unserialize(char* pBuf, tsTick* pTick);

protected:
    friend class tsTickQueue;

    static int serializedTailSize(const tsTick& tick);
    void serializeHead(char* pBuf, int tail) const;
    static void unserializeHead(char* pBuf, tsTick* pTick);
};

struct tsTickDiag : tsTick
{
    bbU64 mSendTime;

    tsTickDiag(const tsObjID& objID) :
        tsTick(objID, tsTickType_Diag),
        mSendTime(0)
    {
    }

    inline bbU64 sendTime() const { return mSendTime; }
    inline void setSendTime(bbU64 timestamp) { mSendTime = timestamp; }
    inline void setSendTime(const tsTime& time) { mSendTime = time.timestamp(); }

    static const int tailSize = 8;
    void serialize(char* pBuf) const;
    void unserialize(const char* pBuf);
    std::string str_tail() const;
};

struct tsTickPrice : tsTick
{
    double mPrice;
    bbU32  mOpt;

    tsTickPrice(const tsObjID& objID, double price, bbU32 opt = 0) :
        tsTick(objID, tsTickType_Price),
        mPrice(price),
        mOpt(opt)
    {
    }

    inline void setPrice(double price) { mPrice = price; }
    inline double price() const { return mPrice; }

    static const int tailSize = 12;
    void serialize(char* pBuf) const {}
    void unserialize(const char* pBuf) {}
};

struct tsTickVolume : tsTick
{
    bbU64  mVolume;

    tsTickVolume(const tsObjID& objID, bbU64 volume) :
        tsTick(objID, tsTickType_Volume),
        mVolume(volume)
    {
    }

    static const int tailSize = 8;
    void serialize(char* pBuf) const {}
    void unserialize(const char* pBuf) {}
};

struct tsTickPriceVolume : tsTick
{
    double mPrice;
    bbU64  mVolume;

    tsTickPriceVolume(const tsObjID& objID, double price, bbU64 volume, bbU64 time=0) :
        tsTick(objID, tsTickType_PriceVolume),
        mPrice(price),
        mVolume(volume)
    {
    }

    static const int tailSize = 16;
    void serialize(char* pBuf) const {}
    void unserialize(const char* pBuf) {}
};

union tsTickUnion
{
    bbU8 mTick[sizeof(tsTick)];
    bbU8 mPrice[sizeof(tsTickPrice)];
    bbU8 mVolume[sizeof(tsTickVolume)];
    bbU8 mPriceVolume[sizeof(tsTickPriceVolume)];
    operator const tsTick*() const { return (const tsTick*)this; }
};

#endif
