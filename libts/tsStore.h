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

    /** Authenticate user.
        @param uid User ID
        @param pPwd Password hash, exactly 32 bytes long
        @return Feed ID (incoming feed) this user is allowed for, 0 for failure, -1 for all allowed
    */
    virtual bbU64 Authenticate(bbU64 uid, const bbU8* pPwd);

    /** Create a new user.
        @param name User name
        @param pPwd Password hash, 32 bytes
        @return User ID
    */
    virtual bbU64 CreateUser(std::string name, const bbU8* pPwd);
};

#endif

