#ifndef tsSTOREMEMORY_H
#define tsSTOREMEMORY_H

#include "tsStore.h"
#include <map>

class tsStoreMemory : public tsStore
{
    typedef std::map<tsObjID, tsHeader*> ObjIDHeaderMap;
    ObjIDHeaderMap mObjID2Header;

public:
    virtual bbERR GetHeader(const tsObjID& objID, tsHeader& header);
    virtual bbERR SetHeader(const tsHeader& header);
    virtual bbERR SaveHeader(tsHeader& header);
};

#endif

