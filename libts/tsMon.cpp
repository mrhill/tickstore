#include "tsMon.h"
#include <math.h>
#include <float.h>

tsMonSymbol::tsMonSymbol()
{
    clear();
}

void tsMonSymbol::clear()
{
    mValid = 0;

    mPrice = 0;
    mLow = 0;
    mHigh = 0;
    mOpen = 0;
    mClose = 0;
    mVolume = 0;
    mTotalVolume = 0;
}

