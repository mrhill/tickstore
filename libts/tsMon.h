#ifndef tsMON_H
#define tsMON_H

#include "tsdef.h"

class tsMon
{
public:
    virtual void clear() = 0;
};

class tsMonSymbol
{
    bbU32  mValid;
    double mPrice;
    double mHigh;
    double mLow;
    double mOpen;
    double mClose;
    bbU64  mVolume;
    bbU64  mTotalVolume;

public:
    tsMonSymbol();

    void clear();

    enum
    {
        Valid_Price = 0x1,
        Valid_High  = 0x2,
        Valid_Low   = 0x4,
        Valid_Open  = 0x8,
        Valid_Close = 0x10
    };

    bbU32 validMask() const { return mValid; }
    int priceValid() const { return mValid & Valid_Price; }
    int highValid() const { return mValid & Valid_High; }
    int lowValid() const { return mValid & Valid_Low; }
    int openValid() const { return mValid & Valid_Open; }
    int closeValid() const { return mValid & Valid_Close; }

    inline void setPrice(double v) { mPrice = v; mValid |= Valid_Price; }
    inline void setHigh(double v)  { mHigh = v;  mValid |= Valid_High; }
    inline void setLow(double v)   { mLow = v;   mValid |= Valid_Low; }
    inline void setOpen(double v)  { mOpen = v;  mValid |= Valid_Open; }
    inline void setClose(double v) { mClose = v; mValid |= Valid_Close; }
    inline void setVolume(bbU64 v) { mVolume = v; }
    inline void setTotalVolume(bbU64 v) { mTotalVolume = v; }
    inline void addTotalVolume(bbU64 v) { mTotalVolume += v; }

    inline double price() const { return mPrice; }
    inline double high() const { return mHigh; }
    inline double low() const { return mLow; }
    inline double open() const { return mOpen; }
    inline double close() const { return mClose; }
    inline bbU64  volume() const { return mVolume; }
    inline bbU64  totalVolume() const { return mTotalVolume; }
};

#endif
