#include "tsStore.h"
#include "tsStoreMemory.h"
#include "tsStoreFile.h"

tsStore* tsStore::Create(tsTickFactory& tickFactory, tsStoreBackend type, const char* pName)
{
    tsStore* pStore = NULL;

    switch(type)
    {
    case tsStoreBackend_Memory: pStore = new tsStoreMemory(tickFactory); break;
    case tsStoreBackend_File:   pStore = new tsStoreFile(tickFactory, pName); break;
    default: break;
    }

    return pStore;
}

