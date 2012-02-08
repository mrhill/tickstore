#ifndef tsSTORE_H
#define tsSTORE_H

#include "tsObjID.h"
#include "tsHeader.h"
#include "tsTick.h"

enum tsStoreBackend
{
    tsStoreBackend_Memory
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

    virtual void SaveTick(const char* pRawTick) = 0;

    virtual bbERR GetHeader(const tsObjID& objID, tsHeader& header) = 0;
    virtual bbERR SetHeader(const tsHeader& header) = 0;
};

#endif

