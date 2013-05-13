#ifndef tsOBJID_H
#define tsOBJID_H

#include <babel/defs.h>
#include <string>
#include <ostream>
#include "tsdef.h"

class tsObjID
{
    bbU64 mSymbolID;
    bbU32 mExchangeID;

public:
    tsObjID(bbU32 exchangeID=0, bbU64 symbolID=0) : mExchangeID(exchangeID), mSymbolID(symbolID) {}

    inline void  setExchangeID(bbU32 exchangeID) { mExchangeID = exchangeID; }
    inline bbU32 exchangeID() const { return mExchangeID; }

    inline void  setSymbolID(bbU64 symbolID) { mSymbolID = symbolID; }
    inline bbU64 symbolID() const { return mSymbolID; }

    inline bool operator<(const tsObjID& other) const
    {
        return (mExchangeID <= other.mExchangeID) && (mSymbolID < other.mSymbolID);
    }

    std::string str() const;
};

inline std::ostream& operator<<(std::ostream& o, const tsObjID& id) { return o<<id.str(); }

#endif

