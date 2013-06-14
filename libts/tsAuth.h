#ifndef tsAUTH_H
#define tsAUTH_H

#include "tsMutex.h"
#include "tsMySQL.h"
#include <map>
#include <stdexcept>

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
    virtual bbU64 Authenticate(bbU64 uid, const bbU8* pPwd);
    virtual bbU64 CreateUser(std::string name, const bbU8* pPwd);
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

    virtual bbU64 Authenticate(bbU64 uid, const bbU8* pPwd);
    virtual bbU64 CreateUser(std::string name, const bbU8* pPwd);
};

#endif

