#include "tsTickQueue.h"
#include "babel/fixmath.h"
#include <string.h>
#include <stdexcept>
#include <streamprintf.h>

tsTickQueue::tsTickQueue(bbUINT size) : mpBuf(NULL), mSize(size), mRd(0), mWr(0)
{
    if (!bbIsPwr2(size))
        throw std::runtime_error(strprintf(__FUNCTION__ ": queue size %d is not power of 2\n", size));

    mpBuf = new char[size];
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
    bbUINT const tickSize = tick.serializedSize();

    if (tickSize >= left)
        return false;

    if ((mWr+tickSize) <= mSize)
    {
        tick.serialize(mpBuf + wr);
    }
    else
    {
        bbUINT firsPart = mSize-wr;

        char wrapBuf[tsTick::serializedSizeMax];
        tick.serialize(wrapBuf);

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

void tsTickQueue::backRaw(BufDesc& desc)
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
    }
    else if (rd==0)
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
}

int tsTickQueue::frontSize()
{
    bbUINT rd = mRd;
    bbUINT wr = mWr;
    bbUINT filled = (wr-rd) & (mSize-1);

    if (filled < tsTick::serializedHeadSize)
        return -1;

    bbUINT tickSize;

    if ((rd + tsTick::serializedPrefix) <= mSize)
    {
        tickSize = tsTick::unserialize(mpBuf+rd, NULL);
    }
    else
    {
        bbUINT firsPart = mSize-rd;
        char wrapBuf[tsTick::serializedHeadSize];
        memcpy(wrapBuf, mpBuf + rd, firsPart);
        memcpy(wrapBuf+firsPart, mpBuf, tsTick::serializedPrefix-firsPart);
        tickSize = tsTick::unserialize(wrapBuf, NULL);
    }

    if ((tickSize > tsTick::serializedSizeMax) || (tickSize < tsTick::serializedHeadSize))
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
            tsTick::unserialize(mpBuf+rd, (tsTick*)pTick);
        }
        else
        {
            bbUINT firsPart = mSize-rd;
            char wrapBuf[tsTick::serializedSizeMax];
            memcpy(wrapBuf, mpBuf + rd, firsPart);
            memcpy(wrapBuf+firsPart, mpBuf, tickSize-firsPart);

            tsTick::unserialize(wrapBuf, (tsTick*)pTick);
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
