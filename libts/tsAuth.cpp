#include <iostream>
#include <algorithm>
#include "tsAuth.h"
#include "tsHash.h"
#include "tsSemaphore.h"
#include "tsTick.h"
#include <babel/StrBuf.h>

void tsUser::Clear()
{
    mUID = 0;
    mPerm = 0;
    mFeeds.clear();
}

void tsUser::serialize(bbU8* d)
{
    bbST64LE(d, mUID); d+=8;
    bbST32LE(d, mPerm); d+=4;
    bbU32 feeds = mFeeds.size();
    bbST32LE(d, feeds); d+=4;
    for(bbUINT i=0; i<feeds; i++)
    {
        bbU64 fid = mFeeds[i];
        bbST64LE(d, fid); d+=8;
    }
}

bool tsUser::unserialize(const bbU8* d, bbUINT bufSize)
{
    if (bufSize < 16)
        return false;
    bbU32 feeds = bbLD32LE(d+12);
    if (bufSize != (16+feeds*8))
        return false;

    mUID = bbLD64LE(d); d+=8;
    mPerm = bbLD32LE(d); d+=8;
    mFeeds.resize(feeds);

    for(bbUINT i=0; i<feeds; i++)
    {
        mFeeds[i] = bbLD64LE(d); d+=8;
    }

    return true;
}

tsAuth* tsAuth::sInstance = NULL;

tsAuth::tsAuth(zmq::context_t& zmq) : mZmq(zmq)
{
    if (sInstance)
        throw std::runtime_error(strprintf("%s: only one tsAuth instance may be active", __FUNCTION__));

    int major, minor, patch;
    zmq_version(&major, &minor, &patch);
    printf("%s: zmq version %d.%d.%d\n", __FUNCTION__, major, minor, patch);

    tsSemaphore ready;
    tsThread::start(&ready);
    if (!ready.wait(1000))
        throw std::runtime_error(strprintf("%s: cannot init worker thread", __FUNCTION__));

    sInstance = this;
}

tsAuth::~tsAuth()
{
    tsThread::cancel();
    tsThread::join();
}

void* tsAuth::run(void* arg)
{
    tsUser user;

    try
    {
        zmq::socket_t socket(mZmq, ZMQ_REP);
        socket.bind("inproc://auth");
        ((tsSemaphore*)arg)->post();
        do
        {
            zmq::message_t msg;
            socket.recv(&msg); //xxx add timeout

            if (msg.size() != 44)
            {
                printf("tsAuth::run: invalid message size %lu\n", msg.size());
                continue;
            }

            const bbU8* d = (const bbU8*)msg.data();
            bbU64 uid = bbLD64LE(d);
            int sessionID = bbLD32LE(d + 8 + 32);

            int authResult = Authenticate(uid, d + 8, user);

            msg.rebuild(4 + (authResult ? user.serializedSize() : 0));
            bbU8* p = (bbU8*)msg.data();
            bbST32LE(p, sessionID);

            if (authResult)
                user.serialize(p + 4);

            socket.send(msg);

        } while (!testCancel());
    }
    catch(std::exception& e)
    {
        printf("tsAuth::run: shutting down, %s\n", e.what());
    }
    printf("tsAuth::run: shutting down\n");
}

int tsAuth::Authenticate(bbU64 uid, const bbU8* pPwd, tsUser& user)
{
    user.mUID = uid;
    user.mPerm = tsUserPerm_TickToAll;
    return 1; // allow all feeds
}

bbU64 tsAuth::CreateUser(std::string name, const bbU8* pPwd, bbU32 perm)
{
    throw tsAuthException("Not implemented");
}

tsAuthMySQL::tsAuthMySQL(zmq::context_t& zmq, const json_value& cfg)
  : tsAuth(zmq), mCon(cfg["dbname"], cfg["host"], (int)cfg["port"], cfg["user"], cfg["pass"])
{
    CreateUserTable();
}

tsAuthMySQL::~tsAuthMySQL()
{
}

void tsAuthMySQL::CreateUserTable()
{
    bbStrBuf sql;
    sql.Printf("CREATE TABLE IF NOT EXISTS user ("
                   "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE KEY,"
                   "pwd BINARY(32) NOT NULL DEFAULT '',"
                   "salt BINARY(32) NOT NULL DEFAULT '',"
                   "name VARCHAR(256) NOT NULL DEFAULT '',"
                   "perm INT UNSIGNED NOT NULL DEFAULT 0)");

    if (mysql_query(mCon, sql.GetPtr()))
        throw tsAuthException(strprintf("%s: %s\n", __FUNCTION__, mysql_error(mCon)));
    else
        printf("%s: Created user table\n", __FUNCTION__);

    sql.Printf("CREATE TABLE IF NOT EXISTS feed ("
                   "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE KEY,"
                   "uid BIGINT UNSIGNED NOT NULL DEFAULT 0,"
                   "name VARCHAR(256) NOT NULL DEFAULT '',"
                   "FOREIGN KEY (id) REFERENCES user(id))");

    if (mysql_query(mCon, sql.GetPtr()))
        throw tsAuthException(strprintf("%s: %s\n", __FUNCTION__, mysql_error(mCon)));
    else
        printf("%s: Created feed table\n", __FUNCTION__);
}

int tsAuthMySQL::Authenticate(bbU64 uid, const bbU8* pPwd, tsUser& user)
{
    user.Clear();

    tsMutexLocker lock(mMutex);

    bbStrBuf sql;
    sql.Printf("SELECT pwd,salt,perm FROM user WHERE id=%"bbI64"u", uid);
    tsMySQLQuery q(mCon, sql.GetPtr());

    // - Check if UID exists
    MYSQL_ROW row = q.FetchRow();
    if (!row)
    {
        printf("%s: unknown UID 0x%"bbI64"X\n", __FUNCTION__, uid);
        return 0;
    }

    if (!row[0] || !row[1] || !row[2] || q.GetFieldLen(0)!=32 || q.GetFieldLen(1)!=32)
        throw tsAuthException(strprintf("%s: unexpected user query result\n", __FUNCTION__));

    // - sha256 the salted input, and compare with stored result
    tsHash hash(MHASH_SHA256);
    hash.update(row[1], 32); // salt
    hash.update(pPwd, 32); // input password

    if (memcmp(hash.digest(), row[0], 32))
    {
        printf("%s: UID 0x%"bbI64"X, password mismatch\n", __FUNCTION__, uid);
        return 0;
    }

    user.mUID = uid;
    user.mPerm = atoi(row[2]);

    if (user.mPerm & tsUserPerm_TickToAll)
    {
        printf("%s: UID 0x%"bbI64"X allowed to tick to all feeds\n", __FUNCTION__, uid);
    }
    else
    {
        sql.Printf("SELECT id FROM feed WHERE uid=%"bbI64"u", uid);
        q.Exec(sql.GetPtr());

        for(;;)
        {
            row = q.FetchRow();
            if (!row)
                break;

            #ifdef _MSC_VER
            bbU64 feedID = _strtoui64(row[0], NULL, 10);
            #else
            bbU64 feedID = strtoull(row[0], NULL, 10);
            #endif
            user.mFeeds.push_back(feedID);
            printf("%s: Allowing feed 0x%"bbI64"X for UID 0x%"bbI64"X\n", __FUNCTION__, feedID, uid);
        }

        if (user.mFeeds.empty())
            printf("%s: No feeds for UID 0x%"bbI64"X\n", __FUNCTION__, uid);
        else
            std::sort(user.mFeeds.begin(), user.mFeeds.end());
    }

    return 1;
}

bbU64 tsAuthMySQL::CreateUser(std::string name, const bbU8* pPwd, bbU32 perm)
{
    tsMutexLocker lock(mMutex);

    bbStrBuf sql("INSERT INTO user (name, pwd, salt, perm) values (\"");

    bbStrBuf escaped;
    if (!escaped.SetLen(name.size() << 1))
        throw tsAuthException(strprintf("%s: out of memory\n", __FUNCTION__));
    mysql_escape_string(escaped.GetPtr(), name.c_str(), name.size());

    sql += escaped.GetPtr();
    sql += "\",\"";

    if (!escaped.SetLen(32*2))
        throw tsAuthException(strprintf("%s: out of memory\n", __FUNCTION__));

    tsHash salt(MHASH_SHA256);
    salt.update(name.c_str(), name.size());
    const char* stat = mysql_stat(mCon);
    salt.update(stat, strlen(stat));
    const char* pSalt = salt.digest();

    tsHash pwdHash(MHASH_SHA256);
    pwdHash.update(pSalt, 32);
    pwdHash.update(pPwd, 32);

    mysql_real_escape_string(mCon, escaped.GetPtr(), pwdHash.digest(), 32);
    sql += escaped.GetPtr();
    sql += "\",\"";


    mysql_real_escape_string(mCon, escaped.GetPtr(), pSalt, 32);
    sql += escaped.GetPtr();

    sql.Catf("\",%u)", perm);

    if (mysql_query(mCon, sql.GetPtr()))
        throw tsAuthException(strprintf("%s: %s\n", __FUNCTION__, mysql_error(mCon)));

    bbU64 uid = mysql_insert_id(mCon);
    printf("%s: Created user %s with uid 0x%"bbI64"X\n", __FUNCTION__, name.c_str(), uid);
    return uid;
}

