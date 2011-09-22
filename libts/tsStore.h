#ifndef tsSTORE_H
#define tsSTORE_H

#include "tsObjID.h"
#include "tsHeader.h"

enum tsStoreBackend
{
    tsStoreBackend_Memory
};

class tsStore
{
public:
    static tsStore* Create(tsStoreBackend type);
    static void Destroy(tsStore* pTS) { delete pTS; }

    virtual bbERR GetHeader(const tsObjID& objID, tsHeader& header) = 0;
    virtual bbERR SetHeader(const tsHeader& header) = 0;
};

#endif

