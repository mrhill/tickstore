#include "tsTickQueue.h"
#include "babel/fixmath.h"
#include <string.h>
#include <stdexcept>
#include <streamprintf.h>

tsTickQueue::tsTickQueue(tsTickFactory& tickFactory, bbUINT bufsize) : mTickFactory(tickFactory), mpBuf(NULL), mSize(bufsize), mRd(0), mWr(0)
{
    if (!bbIsPwr2(bufsize))
        throw std::runtime_error(strprintf(__FUNCTION__ ": queue buffer size %d is not power of 2\n", bufsize));

    mpBuf = new char[bufsize];
}

tsTickQueue::~tsTickQueue()
{
    delete[] mpBuf;
}

bool tsTickQueue::push(const tsTick& tick)
{
    bbUINT rd = mRd;
    bbUINT wr = mWr;
    bbUINT left = mSize - ((wr-rd)&(mSize-1));
    bbUINT const tickSize = mTickFactory.serializedSize(tick);

    if (tickSize >= left)
        return false;

    if ((mWr+tickSize) <= mSize)
    {
        mTickFactory.serialize(tick, mpBuf + wr);
    }
    else
    {
        bbUINT firsPart = mSize-wr;

        char wrapBuf[tsTick::SERIALIZEDMAXSIZE];
        mTickFactory.serialize(tick, wrapBuf);

        memcpy(mpBuf + wr, wrapBuf, firsPart);
        memcpy(mpBuf, wrapBuf + firsPart, tickSize-firsPart);
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
        tickSize = mTickFactory.unserialize(mpBuf+rd, NULL);
    }
    else
    {
        bbUINT firsPart = mSize-rd;
        char wrapBuf[tsTick::SERIALIZEDHEADSIZE];
        memcpy(wrapBuf, mpBuf + rd, firsPart);
        memcpy(wrapBuf+firsPart, mpBuf, tsTick::SERIALIZEDPREFIX-firsPart);
        tickSize = mTickFactory.unserialize(wrapBuf, NULL);
    }

    if ((tickSize > tsTick::SERIALIZEDMAXSIZE) || (tickSize < tsTick::SERIALIZEDHEADSIZE))
    {
        printf(__FUNCTION__ ": Warning, bad ticksize %d\n", tickSize);
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
            mTickFactory.unserialize(mpBuf+rd, (tsTick*)pTick);
        }
        else
        {
            bbUINT firsPart = mSize-rd;
            char wrapBuf[tsTick::SERIALIZEDMAXSIZE];
            memcpy(wrapBuf, mpBuf + rd, firsPart);
            memcpy(wrapBuf+firsPart, mpBuf, tickSize-firsPart);

            mTickFactory.unserialize(wrapBuf, (tsTick*)pTick);
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
