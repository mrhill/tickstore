#ifndef tsTICKQUEUE_H
#define tsTICKQUEUE_H

#include "tsTick.h"
#include <stdio.h>

/** Tick Queue.
    Provides mutex-less but thread-save mechanism to pass serialized tsTick's between
    one producer and one consumer thread. Can also be used as a in-place circular buffer to
    send or receive ticks over a socket or file descriptor.
*/
class tsTickQueue
{
    tsTickFactory& mTickFactory;    //!< Factory for tick serialization
    char*   mpBuf;                  //!< Queue circular buffer
    bbUINT  mRd;                    //!< Current read offset
    bbUINT  mWr;                    //!< Current write offset
    bbUINT  mSize;                  //!< Size of queue buffer, must be power of 2
    FILE*   mLogFD;
    char    mWrapBuf[tsTick::SERIALIZEDMAXSIZE]; //!< Temp buffer to handle wraps
    std::string mName;

public:
    struct BufDesc
    {
        char* pFirst;
        char* pSecond;
        bbUINT sizeFirst;
        bbUINT sizeSecond;
    };

    tsTickQueue(tsTickFactory& tickFactory, const char* pQueueName = NULL, bool logToFile = false, bbUINT bufsize = 8192);
    ~tsTickQueue();

    const char* name() const { return mName.c_str(); }

    /** Test if queue is empty.
        @return true if empty, false if data available
    */
    bool empty() const { return mRd==mWr; }

    /** Return number of bytes in queue. */
    bbUINT size() const { return (mWr-mRd)&(mSize-1); }

    /** Serialize tick and push to queue.
        @param tick Tick to serialize
        @return true if success, false if queue full
    */
    bool push(const tsTick& tick);

    /** Get pointer to free tail of queue.
        The caller can write raw data into the queue and commit by calling pushRaw()
        @param desc Buffer tail descriptor, will be filled on success
        @return true on success if free bytes available, false if queue full
    */
    bool backRaw(BufDesc& desc);

    /** Commit raw bytes to end of queue.
        To be used in combination with backRaw(). Number of bytes pushed can be incomplete ticks.
        @param size Number of bytes to commit
    */
    void pushRaw(bbUINT size);

    /** Return byte size of raw tick at front of queue.
        @return Size in bytes, -1 if no or incomplete data, -2 if malformed data detected
    */
    int  frontSize();

    /** Return pointer to raw tick at front of queue.
        @param ppBuf Returns pointer to raw tick as continuous block
        @return Size in bytes or <=0 if no or incomplete data, -2 if malformed data detected
    */
    int  frontRaw(char** ppBuf);

    /** Unserialize tick at front of queue.
        @param pTick
        @return Size in bytes or <=0 if no or incomplete data, -2 if malformed data detected
    */
    int  front(tsTickUnion* pTick);

    /** Remove tick from front of queue.
        @return true if popped, false if not
    */
    bool pop();

    /** Remove tick from front of queue.
        @param Ticksize in bytes as returned by last call to front()
    */
    inline void pop(bbUINT tickSize)
    {
        bbASSERT((bbUINT)tickSize <= size());
        mRd = (mRd + tickSize) & (mSize-1);
    }

    /** Flush all data in queue.
        Only thread-safe to call from read size.
    */
    inline void flush()
    {
        mRd = mWr;
    }
};

#endif

