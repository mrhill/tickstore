#ifndef tsAUTH_H
#define tsAUTH_H

#include "tsMutex.h"
#include "tsMySQL.h"
#include "tsThread.h"
#include "tsUser.h"
#include <map>
#include <stdexcept>
#include "json.h"
#include <zmq.hpp>

class tsAuthException : public std::runtime_error
{
public:
    tsAuthException(const std::string& what_arg) : std::runtime_error(what_arg) {}
};

class tsAuth : protected tsThread
{
    static tsAuth* sInstance;
    zmq::context_t& mZmq;
    virtual void* run(void*);
public:
    tsAuth(zmq::context_t& zmq);
    virtual ~tsAuth();
    static inline tsAuth& instance() { return *sInstance; }
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
    tsAuthMySQL(zmq::context_t& zmq, const json_value& authConfig);
    virtual ~tsAuthMySQL();

    virtual int Authenticate(bbU64 uid, const bbU8* pPwd, tsUser& user);
    virtual bbU64 CreateUser(std::string name, const bbU8* pPwd, bbU32 perm);
};

#endif

