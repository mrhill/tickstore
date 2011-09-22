#ifndef tsTIME_H
#define tsTIME_H

#include <babel/defs.h>
#include <string>

#ifdef min
#undef min
#endif

class tsTime
{
    bbU64 mTimestamp;
    mutable bbU8  mOpt;  // 0!=fields below valid
    mutable bbU8  mSec;
    mutable bbU8  mMin;
    mutable bbU8  mHour;
    mutable bbU8  mDay;
    mutable bbU8  mMonth;
    mutable bbU16 mYear;

    void decodeTimestamp() const;

public:
    tsTime(const bbU64 timestamp = 0) : mTimestamp(timestamp), mOpt(0) {}
    tsTime(int year, int month, int day, int h, int m, int s, int ns);

    bbU64  timestamp() const { return mTimestamp; }
    bbU32  nsec() const { return (bbU32)(mTimestamp % 1000000000); }
    bbU32  msec() const { return (bbU32)((mTimestamp/1000000) % 1000); }
    bbUINT sec() const;
    bbUINT min() const;
    bbUINT hour() const;
    bbUINT day() const;
    bbUINT month() const;
    bbUINT year() const;

    std::string str() const;

    static bbU64 currentNs();
    static tsTime current() { return tsTime(currentNs()); }
};

class tsDate
{
public:
    tsDate();
    tsDate(int y, int m, int d);
};

#endif
