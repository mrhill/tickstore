/*** This file is generated by makeTick.py, do not edit ***/

#ifndef tsTICKFACTORY_H
#define tsTICKFACTORY_H

/** Finance domain tick types, see tsTickType. */
enum
{
    tsTickTypeBegin0 = tsTickTypeBegin-1,

    tsTickType_AuthReply,
    tsTickType_Subscribe,

    tsTickTypeDomain_Finance0 = tsTickTypeDomain_Finance-1,

    tsTickType_Price,
    tsTickType_Volume,
    tsTickType_PriceVolume,
    tsTickType_Bid,
    tsTickType_Ask,
    tsTickType_BidAsk,
    tsTickType_Recap,

    tsTickTypeDomain_KPI0 = tsTickTypeDomain_KPI-1,

    tsTickType_S32,
    tsTickType_F64,

};

class tsTickFactory
{
    tsTickFactory() {}
protected:
    static int  serializedTailSize(const tsTick& tick);
    static void serializeTail(const tsTick* pTick, char* pBuf);
    static std::string strTail(const tsTick* pTick);
public:
    static void unserializeTail(const char* pBuf, tsTick* pTick);
    static int  serializedSize(const tsTick& tick) { return serializedTailSize(tick) + tsTick::SERIALIZEDHEADSIZE; }
    static int  serialize(const tsTick& tick, char* pBuf);
    static int  unserialize(const char* pBuf, tsTick* pTick);
    static std::string str(const tsTick& tick);
};


struct tsTickAuthReply : tsTick
{
    bbU64 mUID;
    bbU32 mSuccess;

    tsTickAuthReply() :
        mUID(0),
        mSuccess(0),
    tsTick(tsTickType_AuthReply) {}

    tsTickAuthReply(const tsObjID& objID) :
        mUID(0),
        mSuccess(0),
    tsTick(objID, tsTickType_AuthReply) {}

    tsTickAuthReply(const tsObjID& objID, bbU64 UID, bbU32 success) :
        mUID(UID),
        mSuccess(success),
    tsTick(objID, tsTickType_AuthReply) {}

    inline void setUID(bbU64 UID) { mUID = UID; }
    inline bbU64 UID() const { return mUID; }
    inline void setSuccess(bbU32 success) { mSuccess = success; }
    inline bbU32 success() const { return mSuccess; }

    static const int TAILSIZE = 12;
    static const int SERIALIZEDSIZE = SERIALIZEDHEADSIZE + TAILSIZE;
    void serializeTail(char* pBuf) const;
    void unserializeTail(const char* pBuf);
    std::string strTail() const;
};

struct tsTickSubscribe : tsTick
{
    bbU64 mFeedID;

    tsTickSubscribe() :
        mFeedID(0),
    tsTick(tsTickType_Subscribe) {}

    tsTickSubscribe(const tsObjID& objID) :
        mFeedID(0),
    tsTick(objID, tsTickType_Subscribe) {}

    tsTickSubscribe(const tsObjID& objID, bbU64 feedID) :
        mFeedID(feedID),
    tsTick(objID, tsTickType_Subscribe) {}

    inline void setFeedID(bbU64 feedID) { mFeedID = feedID; }
    inline bbU64 feedID() const { return mFeedID; }

    static const int TAILSIZE = 8;
    static const int SERIALIZEDSIZE = SERIALIZEDHEADSIZE + TAILSIZE;
    void serializeTail(char* pBuf) const;
    void unserializeTail(const char* pBuf);
    std::string strTail() const;
};

struct tsTickPrice : tsTick
{
    double mPrice;
    bbU32 mOpt;

    tsTickPrice() :
        mPrice(0),
        mOpt(0),
    tsTick(tsTickType_Price) {}

    tsTickPrice(const tsObjID& objID) :
        mPrice(0),
        mOpt(0),
    tsTick(objID, tsTickType_Price) {}

    tsTickPrice(const tsObjID& objID, double price, bbU32 opt) :
        mPrice(price),
        mOpt(opt),
    tsTick(objID, tsTickType_Price) {}

    inline void setPrice(double price) { mPrice = price; }
    inline double price() const { return mPrice; }
    inline void setOpt(bbU32 opt) { mOpt = opt; }
    inline bbU32 opt() const { return mOpt; }

    static const int TAILSIZE = 12;
    static const int SERIALIZEDSIZE = SERIALIZEDHEADSIZE + TAILSIZE;
    void serializeTail(char* pBuf) const;
    void unserializeTail(const char* pBuf);
    std::string strTail() const;
};

struct tsTickVolume : tsTick
{
    bbU64 mVolume;
    bbU32 mOpt;

    tsTickVolume() :
        mVolume(0),
        mOpt(0),
    tsTick(tsTickType_Volume) {}

    tsTickVolume(const tsObjID& objID) :
        mVolume(0),
        mOpt(0),
    tsTick(objID, tsTickType_Volume) {}

    tsTickVolume(const tsObjID& objID, bbU64 volume, bbU32 opt) :
        mVolume(volume),
        mOpt(opt),
    tsTick(objID, tsTickType_Volume) {}

    inline void setVolume(bbU64 volume) { mVolume = volume; }
    inline bbU64 volume() const { return mVolume; }
    inline void setOpt(bbU32 opt) { mOpt = opt; }
    inline bbU32 opt() const { return mOpt; }

    static const int TAILSIZE = 12;
    static const int SERIALIZEDSIZE = SERIALIZEDHEADSIZE + TAILSIZE;
    void serializeTail(char* pBuf) const;
    void unserializeTail(const char* pBuf);
    std::string strTail() const;
};

struct tsTickPriceVolume : tsTick
{
    double mPrice;
    bbU64 mVolume;
    bbU32 mOpt;

    tsTickPriceVolume() :
        mPrice(0),
        mVolume(0),
        mOpt(0),
    tsTick(tsTickType_PriceVolume) {}

    tsTickPriceVolume(const tsObjID& objID) :
        mPrice(0),
        mVolume(0),
        mOpt(0),
    tsTick(objID, tsTickType_PriceVolume) {}

    tsTickPriceVolume(const tsObjID& objID, double price, bbU64 volume, bbU32 opt) :
        mPrice(price),
        mVolume(volume),
        mOpt(opt),
    tsTick(objID, tsTickType_PriceVolume) {}

    inline void setPrice(double price) { mPrice = price; }
    inline double price() const { return mPrice; }
    inline void setVolume(bbU64 volume) { mVolume = volume; }
    inline bbU64 volume() const { return mVolume; }
    inline void setOpt(bbU32 opt) { mOpt = opt; }
    inline bbU32 opt() const { return mOpt; }

    static const int TAILSIZE = 20;
    static const int SERIALIZEDSIZE = SERIALIZEDHEADSIZE + TAILSIZE;
    void serializeTail(char* pBuf) const;
    void unserializeTail(const char* pBuf);
    std::string strTail() const;
};

struct tsTickBid : tsTick
{
    double mPrice;
    bbU64 mVolume;
    bbU32 mOpt;

    tsTickBid() :
        mPrice(0),
        mVolume(0),
        mOpt(0),
    tsTick(tsTickType_Bid) {}

    tsTickBid(const tsObjID& objID) :
        mPrice(0),
        mVolume(0),
        mOpt(0),
    tsTick(objID, tsTickType_Bid) {}

    tsTickBid(const tsObjID& objID, double price, bbU64 volume, bbU32 opt) :
        mPrice(price),
        mVolume(volume),
        mOpt(opt),
    tsTick(objID, tsTickType_Bid) {}

    inline void setPrice(double price) { mPrice = price; }
    inline double price() const { return mPrice; }
    inline void setVolume(bbU64 volume) { mVolume = volume; }
    inline bbU64 volume() const { return mVolume; }
    inline void setOpt(bbU32 opt) { mOpt = opt; }
    inline bbU32 opt() const { return mOpt; }

    static const int TAILSIZE = 20;
    static const int SERIALIZEDSIZE = SERIALIZEDHEADSIZE + TAILSIZE;
    void serializeTail(char* pBuf) const;
    void unserializeTail(const char* pBuf);
    std::string strTail() const;
};

struct tsTickAsk : tsTick
{
    double mPrice;
    bbU64 mVolume;
    bbU32 mOpt;

    tsTickAsk() :
        mPrice(0),
        mVolume(0),
        mOpt(0),
    tsTick(tsTickType_Ask) {}

    tsTickAsk(const tsObjID& objID) :
        mPrice(0),
        mVolume(0),
        mOpt(0),
    tsTick(objID, tsTickType_Ask) {}

    tsTickAsk(const tsObjID& objID, double price, bbU64 volume, bbU32 opt) :
        mPrice(price),
        mVolume(volume),
        mOpt(opt),
    tsTick(objID, tsTickType_Ask) {}

    inline void setPrice(double price) { mPrice = price; }
    inline double price() const { return mPrice; }
    inline void setVolume(bbU64 volume) { mVolume = volume; }
    inline bbU64 volume() const { return mVolume; }
    inline void setOpt(bbU32 opt) { mOpt = opt; }
    inline bbU32 opt() const { return mOpt; }

    static const int TAILSIZE = 20;
    static const int SERIALIZEDSIZE = SERIALIZEDHEADSIZE + TAILSIZE;
    void serializeTail(char* pBuf) const;
    void unserializeTail(const char* pBuf);
    std::string strTail() const;
};

struct tsTickBidAsk : tsTick
{
    double mPriceBid;
    double mPriceAsk;
    bbU32 mOpt;

    tsTickBidAsk() :
        mPriceBid(0),
        mPriceAsk(0),
        mOpt(0),
    tsTick(tsTickType_BidAsk) {}

    tsTickBidAsk(const tsObjID& objID) :
        mPriceBid(0),
        mPriceAsk(0),
        mOpt(0),
    tsTick(objID, tsTickType_BidAsk) {}

    tsTickBidAsk(const tsObjID& objID, double priceBid, double priceAsk, bbU32 opt) :
        mPriceBid(priceBid),
        mPriceAsk(priceAsk),
        mOpt(opt),
    tsTick(objID, tsTickType_BidAsk) {}

    inline void setPriceBid(double priceBid) { mPriceBid = priceBid; }
    inline double priceBid() const { return mPriceBid; }
    inline void setPriceAsk(double priceAsk) { mPriceAsk = priceAsk; }
    inline double priceAsk() const { return mPriceAsk; }
    inline void setOpt(bbU32 opt) { mOpt = opt; }
    inline bbU32 opt() const { return mOpt; }

    static const int TAILSIZE = 20;
    static const int SERIALIZEDSIZE = SERIALIZEDHEADSIZE + TAILSIZE;
    void serializeTail(char* pBuf) const;
    void unserializeTail(const char* pBuf);
    std::string strTail() const;
};

struct tsTickRecap : tsTick
{
    double mOpen;
    double mHigh;
    double mLow;
    double mClose;
    bbU64 mVolume;
    bbU64 mOpenInt;
    bbU32 mOpt;

    tsTickRecap() :
        mOpen(0),
        mHigh(0),
        mLow(0),
        mClose(0),
        mVolume(0),
        mOpenInt(0),
        mOpt(0),
    tsTick(tsTickType_Recap) {}

    tsTickRecap(const tsObjID& objID) :
        mOpen(0),
        mHigh(0),
        mLow(0),
        mClose(0),
        mVolume(0),
        mOpenInt(0),
        mOpt(0),
    tsTick(objID, tsTickType_Recap) {}

    tsTickRecap(const tsObjID& objID, double open, double high, double low, double close, bbU64 volume, bbU64 openInt, bbU32 opt) :
        mOpen(open),
        mHigh(high),
        mLow(low),
        mClose(close),
        mVolume(volume),
        mOpenInt(openInt),
        mOpt(opt),
    tsTick(objID, tsTickType_Recap) {}

    inline void setOpen(double open) { mOpen = open; }
    inline double open() const { return mOpen; }
    inline void setHigh(double high) { mHigh = high; }
    inline double high() const { return mHigh; }
    inline void setLow(double low) { mLow = low; }
    inline double low() const { return mLow; }
    inline void setClose(double close) { mClose = close; }
    inline double close() const { return mClose; }
    inline void setVolume(bbU64 volume) { mVolume = volume; }
    inline bbU64 volume() const { return mVolume; }
    inline void setOpenInt(bbU64 openInt) { mOpenInt = openInt; }
    inline bbU64 openInt() const { return mOpenInt; }
    inline void setOpt(bbU32 opt) { mOpt = opt; }
    inline bbU32 opt() const { return mOpt; }

    static const int TAILSIZE = 52;
    static const int SERIALIZEDSIZE = SERIALIZEDHEADSIZE + TAILSIZE;
    void serializeTail(char* pBuf) const;
    void unserializeTail(const char* pBuf);
    std::string strTail() const;
};

struct tsTickS32 : tsTick
{
    bbS32 mCount;
    bbU32 mOpt;

    tsTickS32() :
        mCount(0),
        mOpt(0),
    tsTick(tsTickType_S32) {}

    tsTickS32(const tsObjID& objID) :
        mCount(0),
        mOpt(0),
    tsTick(objID, tsTickType_S32) {}

    tsTickS32(const tsObjID& objID, bbS32 count, bbU32 opt) :
        mCount(count),
        mOpt(opt),
    tsTick(objID, tsTickType_S32) {}

    inline void setCount(bbS32 count) { mCount = count; }
    inline bbS32 count() const { return mCount; }
    inline void setOpt(bbU32 opt) { mOpt = opt; }
    inline bbU32 opt() const { return mOpt; }

    static const int TAILSIZE = 8;
    static const int SERIALIZEDSIZE = SERIALIZEDHEADSIZE + TAILSIZE;
    void serializeTail(char* pBuf) const;
    void unserializeTail(const char* pBuf);
    std::string strTail() const;
};

struct tsTickF64 : tsTick
{
    double mValue;
    bbU32 mOpt;

    tsTickF64() :
        mValue(0),
        mOpt(0),
    tsTick(tsTickType_F64) {}

    tsTickF64(const tsObjID& objID) :
        mValue(0),
        mOpt(0),
    tsTick(objID, tsTickType_F64) {}

    tsTickF64(const tsObjID& objID, double value, bbU32 opt) :
        mValue(value),
        mOpt(opt),
    tsTick(objID, tsTickType_F64) {}

    inline void setValue(double value) { mValue = value; }
    inline double value() const { return mValue; }
    inline void setOpt(bbU32 opt) { mOpt = opt; }
    inline bbU32 opt() const { return mOpt; }

    static const int TAILSIZE = 12;
    static const int SERIALIZEDSIZE = SERIALIZEDHEADSIZE + TAILSIZE;
    void serializeTail(char* pBuf) const;
    void unserializeTail(const char* pBuf);
    std::string strTail() const;
};

#endif

