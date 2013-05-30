#ifndef tsSTOREMEMORY_H
#define tsSTOREMEMORY_H

#include "tsStore.h"
#include "tsHeader.h"
#include <map>
#include <vector>

class tsStoreMemory : public tsStore
{
    typedef std::vector<char> Table;
    typedef std::map<tsObjID, Table*> ObjIDTableMap;
    ObjIDTableMap mObjID2Table;

    typedef std::map<tsObjID, tsHeader*> ObjIDHeaderMap;
    ObjIDHeaderMap mObjID2Header;

public:
    virtual void SaveTick(const char* pRawTick, bbUINT tickSize);
};

#endif

