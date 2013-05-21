#ifndef tsSTORE_H
#define tsSTORE_H

#include "tsObjID.h"
#include "tsHeader.h"
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
protected:
    tsTickFactory& mTickFactory;
public:
    static tsStore* Create(tsTickFactory& tickFactory, tsStoreBackend type, const char* pName);
    static void Destroy(tsStore* pTS) { delete pTS; }

    tsStore(tsTickFactory& tickFactory) : mTickFactory(tickFactory) {}
    virtual ~tsStore() {}

    inline tsTickFactory& tickFactory() const { return mTickFactory; }

    virtual void SaveTick(const char* pRawTick, bbUINT tickSize) = 0;
    virtual bbU64 Authenticate(bbU64 uid, const bbU8* pPwd);
};

#endif

