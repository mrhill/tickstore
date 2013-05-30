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

bbU64 tsStore::Authenticate(bbU64 uid, const bbU8* pPwd)
{
    return (bbU64)(bbS64)-1; // allow all feeds
}

bbU64 tsStore::CreateUser(std::string name, const bbU8* pPwd)
{
    throw tsStoreException("Not implemented");
}


