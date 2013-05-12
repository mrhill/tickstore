#ifndef tsTICK_H
#define tsTICK_H

#include "tsObjID.h"
#include "tsTime.h"
#pragma warning(disable: 4355)

enum tsTickType
{
    tsTickType_None = 0,    //!< Indicating unitialized ticktype
    tsTickType_Diag,        //!< Diagnostics tick, see tsTickDiag
    tsTickTypeFirst         //!< First ID for domain specific tick types
};

struct tsTick
{
    tsObjID     mObjID;     //!< Object ID
    bbU16       mType;      //!< Tick type, see tsTickType
    bbU32       mCount;     //!< Tick stream counter, intended for debug
    bbU64       mTime;      //!< Tick timestamp, interpretation depends on tick type

    static const int MAXSIZE = 160; //!< Maximum byte size of tsTick derivated classes, see tsTickUnion
    static const int SERIALIZEDPREFIX = 2+2; // u16 type, u16 serialized size (excl. this word)
    static const int SERIALIZEDHEADSIZE = SERIALIZEDPREFIX+4+8+4+8; // u32 exid, u64 symid, u32 count, u64 time
    static const int SERIALIZEDMAXSIZE = MAXSIZE;

    tsTick(bbU16 type = tsTickType_None) : mType(type), mCount(0), mTime(0) {}
    tsTick(const tsObjID& objID, bbU16 type = tsTickType_None) : mObjID(objID), mType(type), mCount(0), mTime(0) {}

    inline bbU16 type() const { return mType; }

    inline const tsObjID& objID() const { return mObjID; }
    inline void setObjID(const tsObjID& objID) { mObjID=objID; }

    inline bbU32 count() const { return mCount; }
    inline void setCount(bbU32 count) { mCount = count; }

    inline bbU64 time() const { return mTime; }
    inline void setTime(bbU64 timestamp) { mTime = timestamp; }
    inline void setTime(const tsTime& time) { mTime = time.timestamp(); }

    std::string str() const;
    int serializeHead(char* pBuf, int tailSize) const;
    int unserializeHead(const char* pBuf);

    friend class tsTickQueue;
};

struct tsTickDiag : tsTick
{
    bbU64 mSendTime;
    bbU64 mReceiveTime;
    bbU64 mStoreTime;

    tsTickDiag(const tsObjID& objID) :
        tsTick(objID, tsTickType_Diag),
        mSendTime(0),
        mReceiveTime(0),
        mStoreTime(0)
    {
    }

    inline bbU64 sendTime() const { return mSendTime; }
    inline void setSendTime(bbU64 timestamp) { mSendTime = timestamp; }

    inline bbU64 receiveTime() const { return mReceiveTime; }
    inline void setReceiveTime(bbU64 timestamp) { mReceiveTime = timestamp; }

    inline bbU64 storeTime() const { return mStoreTime; }
    inline void setStoreTime(bbU64 timestamp) { mStoreTime = timestamp; }

    static const int tailSize = 24;
    void serializeTail(char* pBuf) const;
    void unserializeTail(const char* pBuf);
    std::string strTail() const;
};

/** Place holder container to unserialize tsTick derived classes. */
union tsTickUnion
{
    bbU8 mTick[tsTick::MAXSIZE];

    operator const tsTick&() const { return *(const tsTick*)this; }
    operator tsTick&() { return *(tsTick*)this; }
};

class tsTickFactory
{
protected:
    virtual int  serializedTailSize(const tsTick& tick) const;
    virtual void serializeTail(const tsTick* pTick, char* pBuf) const;
    virtual void unserializeTail(const char* pBuf, tsTick* pTick) const;
    virtual std::string strTail(const tsTick* pTick) const;
public:
    int         serializedSize(const tsTick& tick) const { return serializedTailSize(tick) + tsTick::SERIALIZEDHEADSIZE; }
    void        serialize(const tsTick& tick, char* pBuf) const;
    int         unserialize(const char* pBuf, tsTick* pTick) const;
    std::string str(const tsTick& tick) const;
};

#endif

