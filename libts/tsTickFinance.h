/*** This file is generated by makeTickFinance.py, do not edit ***/

#ifndef tsTICKFINANCE_H
#define tsTICKFINANCE_H

#include "tsTick.h"
#include <stdio.h>

/** Finance domain tick types, see tsTickType. */
enum
{
    tsTickTypeLast = tsTickTypeFirst-1,

    tsTickType_Price,
    tsTickType_Volume,
    tsTickType_PriceVolume,

    tsTickTypeCount
};

class tsTickFactoryFinance : public tsTickFactory
{
    virtual int  serializedTailSize(const tsTick& tick) const;
    virtual void serializeTail(const tsTick* pTick, char* pBuf) const;
    virtual void unserializeTail(const char* pBuf, tsTick* pTick) const;
    virtual std::string strTail(const tsTick* pTick) const;
};


struct tsTickPrice : tsTick
{
    double mPrice;
    bbU32 mOpt;

    tsTickPrice(const tsObjID& objID, double price, bbU32 opt) :
        mPrice(price),
        mOpt(opt),
        tsTick(objID, tsTickType_Price)
    {
    }


    inline void setPrice(double price) { mPrice = price; }
    inline double price() const { return mPrice; }
    inline void setOpt(bbU32 opt) { mOpt = opt; }
    inline bbU32 opt() const { return mOpt; }

    static const int tailSize = 12;
    void serializeTail(char* pBuf) const;
    void unserializeTail(const char* pBuf);
    std::string strTail() const;
};

struct tsTickVolume : tsTick
{
    bbU64 mVolume;
    bbU32 mOpt;

    tsTickVolume(const tsObjID& objID, bbU64 volume, bbU32 opt) :
        mVolume(volume),
        mOpt(opt),
        tsTick(objID, tsTickType_Volume)
    {
    }


    inline void setVolume(bbU64 volume) { mVolume = volume; }
    inline bbU64 volume() const { return mVolume; }
    inline void setOpt(bbU32 opt) { mOpt = opt; }
    inline bbU32 opt() const { return mOpt; }

    static const int tailSize = 12;
    void serializeTail(char* pBuf) const;
    void unserializeTail(const char* pBuf);
    std::string strTail() const;
};

struct tsTickPriceVolume : tsTick
{
    double mPrice;
    bbU64 mVolume;
    bbU32 mOpt;

    tsTickPriceVolume(const tsObjID& objID, double price, bbU64 volume, bbU32 opt) :
        mPrice(price),
        mVolume(volume),
        mOpt(opt),
        tsTick(objID, tsTickType_PriceVolume)
    {
    }


    inline void setPrice(double price) { mPrice = price; }
    inline double price() const { return mPrice; }
    inline void setVolume(bbU64 volume) { mVolume = volume; }
    inline bbU64 volume() const { return mVolume; }
    inline void setOpt(bbU32 opt) { mOpt = opt; }
    inline bbU32 opt() const { return mOpt; }

    static const int tailSize = 20;
    void serializeTail(char* pBuf) const;
    void unserializeTail(const char* pBuf);
    std::string strTail() const;
};

#endif

