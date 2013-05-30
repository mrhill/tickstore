#include "tsTickQueue.h"
#include "babel/fixmath.h"
#include <string.h>
#include <stdexcept>

tsTickQueue::tsTickQueue(const char* pQueueName, bool logToFile, bbUINT bufsize)
  : mpBuf(NULL), mSize(bufsize), mRd(0), mWr(0), mLogFD(NULL)
{
    if (!bbIsPwr2(bufsize))
        throw std::runtime_error(strprintf("%s: queue buffer size %d is not power of 2\n", __FUNCTION__, bufsize));

    mpBuf = new char[bufsize];

    if (pQueueName)
    {
        mName = pQueueName;
        if (logToFile)
            mLogFD = ::fopen((mName + ".q.dat").c_str(), "ab");
    }
}

tsTickQueue::~tsTickQueue()
{
    if (mLogFD)
        ::fclose(mLogFD);

    delete[] mpBuf;
}

bool tsTickQueue::push(const tsTick& tick)
{
    bbUINT rd = mRd;
    bbUINT wr = mWr;
    bbUINT left = mSize - ((wr-rd)&(mSize-1));
    bbUINT const tickSize = tsTickFactory::serializedSize(tick);

    if (tickSize >= left)
        return false;

    if ((mWr+tickSize) <= mSize)
    {
        tsTickFactory::serialize(tick, mpBuf + wr);

        if (mLogFD)
            fwrite(mpBuf + wr, tickSize, 1, mLogFD);
    }
    else
    {
        bbUINT firstPart = mSize-wr;

        char wrapBuf[tsTick::SERIALIZEDMAXSIZE];
        tsTickFactory::serialize(tick, wrapBuf);

        memcpy(mpBuf + wr, wrapBuf, firstPart);
        memcpy(mpBuf, wrapBuf + firstPart, tickSize-firstPart);

        if (mLogFD)
            fwrite(wrapBuf, tickSize, 1, mLogFD);
    }

    mWr = (wr + tickSize) & (mSize-1);
    return true;
}

void tsTickQueue::pushRaw(bbUINT tickSize)
{
    bbUINT rd = mRd;
    bbUINT wr = mWr;
    bbASSERT(tickSize < (mSize - ((wr-rd)&(mSize-1))));
    mWr = (wr + tickSize) & (mSize-1);
}

bool tsTickQueue::backRaw(BufDesc& desc)
{
    bbUINT rd = mRd; // copy to local to prevent race conditions
    bbUINT wr = mWr;
    bbUINT left = mSize - ((wr-rd)&(mSize-1));

    if (left <= 1)
    {
        bbASSERT(left == 1);
        desc.pFirst     = NULL;
        desc.sizeFirst  = 0;
        desc.pSecond    = NULL;
        desc.sizeSecond = 0;
        return false;
    }

    if (rd==0)
    {
        // |---|.......|
        // rd  wr
        desc.pFirst     = mpBuf + wr;
        desc.sizeFirst  = mSize - wr - 1;
        desc.pSecond    = NULL;
        desc.sizeSecond = 0;
    }
    else if (wr < rd)
    {
        // |---|....|---|
        //     wr   rd
        desc.pFirst     = mpBuf + wr;
        desc.sizeFirst  = rd - wr - 1;
        desc.pSecond    = NULL;
        desc.sizeSecond = 0;
    }
    else if (wr == mSize)
    {
        // |...|--------|
        //     rd   wr
        desc.pFirst     = mpBuf;
        desc.sizeFirst  = rd - 1;
        desc.pSecond    = NULL;
        desc.sizeSecond = 0;
    }
    else
    {
        // |...|----|....|
        //     rd   wr
        desc.pFirst     = mpBuf + wr;
        desc.sizeFirst  = mSize - wr;
        desc.pSecond    = mpBuf;
        desc.sizeSecond = rd - 1;
    }

    return true;
}

int tsTickQueue::frontSize()
{
    bbUINT rd = mRd;
    bbUINT wr = mWr;
    bbUINT filled = (wr-rd) & (mSize-1);

    if (filled < tsTick::SERIALIZEDHEADSIZE)
        return -1;

    bbUINT tickSize;

    if ((rd + tsTick::SERIALIZEDPREFIX) <= mSize)
    {
        tickSize = tsTickFactory::unserialize(mpBuf+rd, NULL);
    }
    else
    {
        bbUINT firsPart = mSize-rd;
        char wrapBuf[tsTick::SERIALIZEDHEADSIZE];
        memcpy(wrapBuf, mpBuf + rd, firsPart);
        memcpy(wrapBuf+firsPart, mpBuf, tsTick::SERIALIZEDPREFIX-firsPart);
        tickSize = tsTickFactory::unserialize(wrapBuf, NULL);
    }

    if ((tickSize > tsTick::SERIALIZEDMAXSIZE) || (tickSize < tsTick::SERIALIZEDHEADSIZE))
    {
        printf("%s: Warning, bad ticksize %d\n", __FUNCTION__, tickSize);
        return -2;
    }

    if (filled < tickSize)
        return -1;

    return tickSize;
}

int tsTickQueue::front(tsTickUnion* pTick)
{
    bbUINT rd = mRd;
    bbUINT wr = mWr;

    int tickSize = frontSize();

    if (tickSize > 0)
    {
        if ((rd+tickSize) <= mSize)
        {
            tsTickFactory::unserialize(mpBuf+rd, (tsTick*)pTick);
        }
        else
        {
            bbUINT firsPart = mSize-rd;
            char wrapBuf[tsTick::SERIALIZEDMAXSIZE];
            memcpy(wrapBuf, mpBuf + rd, firsPart);
            memcpy(wrapBuf+firsPart, mpBuf, tickSize-firsPart);

            tsTickFactory::unserialize(wrapBuf, (tsTick*)pTick);
        }
    }

    return tickSize;
}

int tsTickQueue::frontRaw(char** ppBuf)
{
    bbUINT rd = mRd;
    bbUINT wr = mWr;

    int tickSize = frontSize();

    if (tickSize > 0)
    {
        if ((rd+tickSize) <= mSize)
        {
            *ppBuf = mpBuf+mRd;
        }
        else
        {
            bbUINT firsPart = mSize-rd;
            memcpy(mWrapBuf, mpBuf + rd, firsPart);
            memcpy(mWrapBuf+firsPart, mpBuf, tickSize-firsPart);
            *ppBuf = mWrapBuf;
        }
    }

    return tickSize;
}

bool tsTickQueue::pop()
{
    int tickSize = frontSize();
    if (tickSize <= 0)
        return false;

    bbASSERT((bbUINT)tickSize <= size());

    mRd = (mRd + tickSize) & (mSize-1);
    return true;
}
