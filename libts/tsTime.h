#ifndef tsTIME_H
#define tsTIME_H

#include <babel/defs.h>
#include <string>
#include <ostream>

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
    tsTime(int year, int month, int day, int h = 0, int m = 0, int s = 0, int ns = 0);

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

    static bbU64  currentNs();
    static bbU64  currentMs();
    static tsTime current() { return tsTime(currentNs()); }
};

class tsDate
{
public:
    tsDate();
    tsDate(int y, int m, int d);
};

inline std::ostream& operator<<(std::ostream& o, const tsTime& t) { return o<<t.str(); }

#endif
