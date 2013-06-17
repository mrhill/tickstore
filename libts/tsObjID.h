#ifndef tsOBJID_H
#define tsOBJID_H

#include <ostream>
#include "tsdef.h"

class tsObjID
{
    bbU64 mFeedID;
    bbU64 mSymbolID;

public:
    tsObjID(bbU64 feedID=0, bbU64 symbolID=0) : mFeedID(feedID), mSymbolID(symbolID) {}

    inline void  setFeedID(bbU64 feedID) { mFeedID = feedID; }
    inline bbU64 feedID() const { return mFeedID; }

    inline void  setSymbolID(bbU64 symbolID) { mSymbolID = symbolID; }
    inline bbU64 symbolID() const { return mSymbolID; }

    inline bool operator<(const tsObjID& other) const
    {
        return (mFeedID <= other.mFeedID) && (mSymbolID < other.mSymbolID);
    }

    std::string str() const;
};

inline std::ostream& operator<<(std::ostream& o, const tsObjID& id) { return o<<id.str(); }

#endif

