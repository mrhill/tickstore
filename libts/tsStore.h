#ifndef tsSTORE_H
#define tsSTORE_H

#include "tsTick.h"
#include <stdexcept>

enum tsStoreBackend
{
    tsStoreBackend_Memory,
    tsStoreBackend_File,
    tsStoreBackend_MySQL,
    tsStoreBackendCount
};

class tsStoreException : public std::runtime_error
{
public:
    tsStoreException(const std::string& what_arg) : std::runtime_error(what_arg) {}
};

class tsStore
{
public:
    static tsStore* Create(tsStoreBackend type, const char* pName);
    static void Destroy(tsStore* pTS) { delete pTS; }

    virtual ~tsStore() {}

    virtual void SaveTick(const char* pRawTick, bbUINT tickSize) = 0;
};

#endif

