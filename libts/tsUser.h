#ifndef tsUSER_H
#define tsUSER_H

#include "tsdef.h"
#include <vector>

enum tsUserPerm
{
    tsUserPerm_TickToAll = 0x1,
    tsUserPermCount
};

class tsUser
{
protected:
    bbU64 mUID;
    bbU32 mPerm;
    std::vector<bbU64> mFeeds;

    friend class tsAuth;
    friend class tsAuthMySQL;

public:
    tsUser(bbU64 uid = 0, bbU32 perm = 0) : mUID(uid), mPerm(perm) {}
    void Clear();

    inline bbU64 uid() const { return mUID; }
    inline bbU32 perm() const { return mPerm; }
    inline const std::vector<bbU64>& feeds() const { return mFeeds; }

    inline bbUINT serializedSize() const { return 8+4+4+(mFeeds.size()<<3); }
    void serialize(char* buf);
    bool unserialize(const char* buf, bbUINT bufSize);
};

#endif
