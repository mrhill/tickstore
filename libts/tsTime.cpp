#include <sys/timeb.h>
#include <time.h>
#include "tsdef.h"
#include "tsTime.h"

tsTime::tsTime(int year, int month, int day, int h, int m, int s, int ns)
{
    struct tm tm;
    tm.tm_sec = s;
    tm.tm_min = m;
    tm.tm_hour = h;
    tm.tm_mday = day;
    tm.tm_mon = month - 1;
    tm.tm_year = year - 1900;
    tm.tm_wday = 0;
    tm.tm_yday = 0;
    tm.tm_isdst = 0;

    #ifdef _WIN32
    __time64_t time64 = _mkgmtime64(&tm);
    #else
    time_t t = mktime(&tm);
    #endif
    mTimestamp = (bbU64)t * 1000000000 + ns;
}

void tsTime::decodeTimestamp() const
{
    struct tm tm;

    #ifdef _WIN32
    __time64_t time64 = mTimestamp/1000000000;
    _gmtime64_s(&tm, &time64);
    #else
    time_t t = (time_t)(mTimestamp/1000000000);
    gmtime_r(&t, &tm);
    #endif

    mSec  = tm.tm_sec;
    mMin  = tm.tm_min;
    mHour = tm.tm_hour;
    mDay  = tm.tm_mday;
    mMonth = tm.tm_mon + 1;
    mYear  = tm.tm_year + 1900;
}

bbUINT tsTime::sec() const
{
    if (!mOpt)
        decodeTimestamp();
    return mSec;
}

bbUINT tsTime::min() const
{
    if (!mOpt)
        decodeTimestamp();
    return mMin;
}

bbUINT tsTime::hour() const
{
    if (!mOpt)
        decodeTimestamp();
    return mHour;
}

bbUINT tsTime::day() const
{
    if (!mOpt)
        decodeTimestamp();
    return mDay;
}

bbUINT tsTime::month() const
{
    if (!mOpt)
        decodeTimestamp();
    return mMonth;
}

bbUINT tsTime::year() const
{
    if (!mOpt)
        decodeTimestamp();
    return mYear;
}

std::string tsTime::str() const
{
    return strprintf("%04d%02d%02d-%02d:%02d:%02d-%d", year(), month(), day(), hour(), min(), sec(), msec());
}

bbU64 tsTime::currentNs()
{
    #ifdef _WIN32
    struct __timeb64 timebuffer;
    _ftime64_s(&timebuffer);
    #else
    struct timeb timebuffer;
    ftime(&timebuffer);
    #endif

    return (bbU64)timebuffer.time*1000000000 + ((bbU32)timebuffer.millitm*1000000);
}

bbU64 tsTime::currentMs()
{
    #ifdef _WIN32
    struct __timeb64 timebuffer;
    _ftime64_s( &timebuffer );
    #else
    struct timeb timebuffer;
    ftime(&timebuffer);
    #endif

    return (bbU64)timebuffer.time*1000 + (bbU32)timebuffer.millitm;
}
