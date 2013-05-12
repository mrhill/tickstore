#include "tsStore.h"
#include "tsStoreMemory.h"
#include "tsStoreFile.h"
#include "tsStoreMySQL.h"

tsStore* tsStore::Create(tsTickFactory& tickFactory, tsStoreBackend type, const char* pName)
{
    tsStore* pStore = NULL;

    switch(type)
    {
    case tsStoreBackend_Memory: pStore = new tsStoreMemory(tickFactory); break;
    case tsStoreBackend_File:   pStore = new tsStoreFile(tickFactory, pName); break;
    case tsStoreBackend_MySQL:  pStore = new tsStoreMySQL(tickFactory, pName); break;
    default: break;
    }

    return pStore;
}

