#include "tsUser.h"

void tsUser::Clear()
{
    mUID = 0;
    mPerm = 0;
    mFeeds.clear();
}

void tsUser::serialize(char* d)
{
    bbST64LE(d, mUID); d+=8;
    bbST32LE(d, mPerm); d+=4;
    bbU32 feeds = mFeeds.size();
    bbST32LE(d, feeds); d+=4;
    for(bbUINT i=0; i<feeds; i++)
    {
        bbU64 fid = mFeeds[i];
        bbST64LE(d, fid); d+=8;
    }
}

bool tsUser::unserialize(const char* d, bbUINT bufSize)
{
    if (bufSize < 16)
        return false;
    bbU32 feeds = bbLD32LE(d+12);
    if (bufSize != (16+feeds*8))
        return false;

    mUID = bbLD64LE(d); d+=8;
    mPerm = bbLD32LE(d); d+=8;
    mFeeds.resize(feeds);

    for(bbUINT i=0; i<feeds; i++)
    {
        mFeeds[i] = bbLD64LE(d); d+=8;
    }

    return true;
}

