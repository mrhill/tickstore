#ifndef tsSTORE_H
#define tsSTORE_H

#include "tsObjID.h"
#include "tsHeader.h"
#include "tsTick.h"

enum tsStoreBackend
{
    tsStoreBackend_Memory,
    tsStoreBackend_File,
    tsStoreBackendCount
};

class tsStore
{
protected:
    tsTickFactory& mTickFactory;
public:
    static tsStore* Create(tsTickFactory& tickFactory, tsStoreBackend type);
    static void Destroy(tsStore* pTS) { delete pTS; }
    tsStore(tsTickFactory& tickFactory) : mTickFactory(tickFactory) {}
    inline tsTickFactory& tickFactory() const { return mTickFactory; }

    virtual void SaveTick(const char* pRawTick, bbUINT tickSize) = 0;
};

#endif

