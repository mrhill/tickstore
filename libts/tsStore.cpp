#include "tsStore.h"
#include "tsStoreMemory.h"
#include "tsStoreFile.h"
#include "tsStoreMySQL.h"

tsStore* tsStore::Create(tsStoreBackend type, const char* pName)
{
    tsStore* pStore = NULL;

    switch(type)
    {
    case tsStoreBackend_Memory: pStore = new tsStoreMemory(); break;
    case tsStoreBackend_File:   pStore = new tsStoreFile(pName); break;
    case tsStoreBackend_MySQL:  pStore = new tsStoreMySQL(pName); break;
    default: break;
    }

    return pStore;
}

