#include "tsStore.h"
#include "tsStoreMemory.h"

tsStore* tsStore::Create(tsTickFactory& tickFactory, tsStoreBackend type)
{
    tsStore* pStore = NULL;

    switch(type)
    {
    case tsStoreBackend_Memory: pStore = new tsStoreMemory(tickFactory); break;
    default: break;
    }

    return pStore;
}
