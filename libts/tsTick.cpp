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

void tsTickAuth::serializeTail(char* pBuf) const
{
    bbST64LE(pBuf, mUID); pBuf+=8;
    memcpy(pBuf, mPwdHash, sizeof(mPwdHash));
}

void tsTickAuth::unserializeTail(const char* pBuf)
{
    mUID = bbLD64LE(pBuf); pBuf+=8;
    memcpy(mPwdHash, pBuf, sizeof(mPwdHash));
}

std::string tsTickAuth::strTail() const
{
    std::string str = strprintf(",uid=0x%"bbI64"X,pwd=", mUID);

    for(int i=0; i<sizeof(mPwdHash); i++)
    {
        static const char digits[] = "0123456789ABCDEF";
        str += digits[mPwdHash[i]>>4];
        str += digits[mPwdHash[i]&15];
    }
    return str;
}

void tsTickSubscribe::serializeTail(char* pBuf) const
{
    bbST64LE(pBuf, mFeedID);
}

void tsTickSubscribe::unserializeTail(const char* pBuf)
{
    mFeedID = bbLD64LE(pBuf);
}

std::string tsTickSubscribe::strTail() const
{
    return strprintf(",feed=0x%"bbI64"X", mFeedID);
}
