#include "tsAuth.h"
#include "tsHash.h"
#include <babel/StrBuf.h>

tsAuth* tsAuth::sInstance = NULL;

tsAuth::tsAuth()
{
    if (sInstance)
        throw std::runtime_error("Only one tsAuth instance may be active");
    sInstance = this;
}

bbU64 tsAuth::Authenticate(bbU64 uid, const bbU8* pPwd)
{
    return (bbU64)(bbS64)-1; // allow all feeds
}

bbU64 tsAuth::CreateUser(std::string name, const bbU8* pPwd)
{
    throw tsAuthException("Not implemented");
}

tsAuthMySQL::tsAuthMySQL(const char* pDBName)
  : mCon(pDBName)
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
                   "name VARCHAR(256) NOT NULL DEFAULT '')");

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

bbU64 tsAuthMySQL::Authenticate(bbU64 uid, const bbU8* pPwd)
{
    try
    {
        tsMutexLocker lock(mMutex);

        bbStrBuf sql;
        sql.Printf("SELECT pwd,salt FROM user WHERE id=%"bbI64"u", uid);
        tsMySQLQuery q(mCon, sql.GetPtr());

        // - Check if UID exists
        MYSQL_ROW row = q.FetchRow();
        if (!row)
        {
            printf("%s: unknown UID 0x%"bbI64"X\n", __FUNCTION__, uid);
            return 0;
        }

        if (!row[0] || !row[1] || q.GetFieldLen(0)!=32 || q.GetFieldLen(1)!=32)
            throw tsAuthException(strprintf("%s: unexpected user query result\n", __FUNCTION__));

        // - sha256 the salted input, and compare with stored result
        try
        {
            tsHash hash(MHASH_SHA256);
            hash.update(row[1], 32); // salt
            hash.update(pPwd, 32); // input password

            if (memcmp(hash.digest(), row[0], 32))
            {
                printf("%s: UID 0x%"bbI64"X, password mismatch\n", __FUNCTION__, uid);
                return 0;
            }
        }
        catch(const tsHashException& e)
        {
            throw tsAuthException(e.what());
        }

        sql.Printf("SELECT id FROM feed WHERE uid=%"bbI64"u", uid);
        row = q.ExecAndFetchRow(sql.GetPtr());
        if (!row)
        {
            printf("%s: No feeds for UID 0x%"bbI64"X\n", __FUNCTION__, uid);
            return 0;
        }

        bbU64 feedID = strtoull(row[0], NULL, 10); // _strtoui64() in MSVC
        printf("%s: Allowing feed 0x%"bbI64"X for UID 0x%"bbI64"X\n", __FUNCTION__, feedID, uid);
        return feedID;
    }
    catch(const tsMySQLException& e)
    {
        throw tsAuthException(e.what());
    }
}

bbU64 tsAuthMySQL::CreateUser(std::string name, const bbU8* pPwd)
{
    tsMutexLocker lock(mMutex);

    bbStrBuf sql("INSERT INTO user (name, pwd, salt) values (\"");

    bbStrBuf escaped;
    if (!escaped.SetLen(name.size()*2))
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
    sql += "\")";

    if (mysql_query(mCon, sql.GetPtr()))
        throw tsAuthException(strprintf("%s: %s\n", __FUNCTION__, mysql_error(mCon)));

    bbU64 uid = mysql_insert_id(mCon);
    printf("%s: Created user %s with uid 0x%"bbI64"X\n", __FUNCTION__, name.c_str(), uid);
    return uid;
}


