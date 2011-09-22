#include "tsStore.h"
#include "tsStoreMemory.h"

tsStore* tsStore::Create(tsStoreBackend type)
{
    switch(type)
    {
    case tsStoreBackend_Memory:
        return new tsStoreMemory;
    default:
        return NULL;
    }
}
