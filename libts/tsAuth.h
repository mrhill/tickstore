#ifndef tsAUTH_H
#define tsAUTH_H

#include "tsMutex.h"
#include "tsMySQL.h"
#include <map>
#include <vector>
#include <stdexcept>

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

    inline bbU32 perm() const { return mPerm; }
    inline const std::vector<bbU64>& feeds() const { return mFeeds; }
};

class tsAuthException : public std::runtime_error
{
public:
    tsAuthException(const std::string& what_arg) : std::runtime_error(what_arg) {}
};

class tsAuth
{
    static tsAuth* sInstance;
public:
    tsAuth();
    static inline tsAuth& instance() { return *sInstance; }
    virtual ~tsAuth() {}
    virtual int Authenticate(bbU64 uid, const bbU8* pPwd, tsUser& user);
    virtual bbU64 CreateUser(std::string name, const bbU8* pPwd, bbU32 perm);
};

class tsAuthMySQL : public tsAuth
{
    tsMySQLCon mCon;
    tsMutex mMutex;

    struct Feed
    {
        bbU64       mFeedID;
        MYSQL_STMT* mInsertStmt;
        MYSQL_BIND  mInsertParam[5];

        Feed(tsAuthMySQL& parent, bbU64 feedID);
        ~Feed();
    };

    typedef std::map<bbU64, Feed*> FeedMap;
    FeedMap mFeedMap;

    void CreateUserTable();

public:
    tsAuthMySQL(const char* pDBName);
    virtual ~tsAuthMySQL();

    virtual int Authenticate(bbU64 uid, const bbU8* pPwd, tsUser& user);
    virtual bbU64 CreateUser(std::string name, const bbU8* pPwd, bbU32 perm);
};

#endif

