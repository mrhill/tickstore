#ifndef tsHASH_H_
#define tsHASH_H_

#include <mhash.h>
#include <stdexcept>

class tsHashException : public std::runtime_error
{
public:
    tsHashException(const std::string& what_arg) : std::runtime_error(what_arg) {}
};

class tsHash
{
    MHASH mTd;
    char mDigest[512/8];

public:
    tsHash(hashid id)
    {
        if ((mTd = mhash_init(id)) == MHASH_FAILED)
            throw tsHashException(strprintf("%s: mhash_init(%d) failed\n", __FUNCTION__, id));
    }

    ~tsHash()
    {
        if (mTd != MHASH_FAILED)
            mhash_deinit(mTd, mDigest);
    }

    bool update(const void* plaintext, unsigned size)
    {
        return mhash(mTd, plaintext, size);
    }

    const char* digest()
    {
        mhash_deinit(mTd, mDigest);
        mTd = MHASH_FAILED;
        return mDigest;
    }
};

#endif


