#ifndef tsSTORE_H
#define tsSTORE_H

#include "tsObjID.h"
#include "tsHeader.h"
#include "tsTick.h"

enum tsStoreBackend
{
    tsStoreBackend_Memory,
    tsStoreBackend_File,
    tsStoreBackend_MySQL,
    tsStoreBackendCount
};

class tsStore
{
protected:
    tsTickFactory& mTickFactory;
public:
    static tsStore* Create(tsTickFactory& tickFactory, tsStoreBackend type, const char* pName);
    static void Destroy(tsStore* pTS) { delete pTS; }

    tsStore(tsTickFactory& tickFactory) : mTickFactory(tickFactory) {}
    virtual ~tsStore() {}

    inline tsTickFactory& tickFactory() const { return mTickFactory; }

    virtual void SaveTick(const char* pRawTick, bbUINT tickSize) = 0;
};

#endif

