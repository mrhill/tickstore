#include "tsStoreMySQL.h"
#include "tsHash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <babel/StrBuf.h>

tsStoreMySQL::tsStoreMySQL(const char* pDBName)
  : mCon(NULL)
{
    printf("%s: MySQL client version: %s\n", __FUNCTION__, mysql_get_client_info());

    mCon = mysql_init(NULL);
    if (!mCon)
        throw tsStoreException(strprintf("%s: Error on mysql_init()\n", __FUNCTION__));

    if (!mysql_real_connect(mCon, "localhost", "", "", NULL, 0, NULL, 0))
        throw tsStoreException(strprintf("%s: %s\n", __FUNCTION__, mysql_error(mCon)));

    printf("%s: MySQL host: %s\n", __FUNCTION__, mysql_get_host_info(mCon));

    if (mysql_query(mCon, strprintf("CREATE DATABASE IF NOT EXISTS %s", pDBName).c_str()) ||
        mysql_select_db(mCon, pDBName))
        throw tsStoreException(strprintf("%s: %s\n", __FUNCTION__, mysql_error(mCon)));
    else
        printf("Selected database %s\n", pDBName);

    CreateUserTable();
}

tsStoreMySQL::~tsStoreMySQL()
{
    for (FeedMap::iterator it = mFeedMap.begin(); it != mFeedMap.end(); it++)
        delete it->second;

    printf("%s: Closing MySQL connection\n", __FUNCTION__);
    mysql_close(mCon);
}

void tsStoreMySQL::CreateUserTable()
{
    bbStrBuf sql;
    sql.Printf("CREATE TABLE IF NOT EXISTS user ("
                   "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE KEY,"
                   "pwd BINARY(32) NOT NULL DEFAULT '',"
                   "salt BINARY(32) NOT NULL DEFAULT '',"
                   "name VARCHAR(256) NOT NULL DEFAULT '')");

    if (mysql_query(mCon, sql.GetPtr()))
        throw tsStoreException(strprintf("%s: %s\n", __FUNCTION__, mysql_error(mCon)));
    else
        printf("%s: Created user table\n", __FUNCTION__);

    sql.Printf("CREATE TABLE IF NOT EXISTS feed ("
                   "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE KEY,"
                   "uid BIGINT UNSIGNED NOT NULL DEFAULT 0,"
                   "name VARCHAR(256) NOT NULL DEFAULT '',"
                   "FOREIGN KEY (id) REFERENCES user(id))");

    if (mysql_query(mCon, sql.GetPtr()))
        throw tsStoreException(strprintf("%s: %s\n", __FUNCTION__, mysql_error(mCon)));
    else
        printf("%s: Created feed table\n", __FUNCTION__);
}

void tsStoreMySQL::CreateFeedTable(bbU64 feedID)
{
    bbStrBuf sql;
    sql.Printf("CREATE TABLE IF NOT EXISTS x%08X%08X ("
                   "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE KEY,"
                   "sym BIGINT UNSIGNED NOT NULL DEFAULT 0,"
                   "tt SMALLINT UNSIGNED NOT NULL DEFAULT 0,"
                   "count INT UNSIGNED NOT NULL DEFAULT 0,"
                   "time BIGINT UNSIGNED NOT NULL DEFAULT 0,"
                   "data VARBINARY(%u) NOT NULL DEFAULT '')", (bbU32)(feedID>>32), (bbU32)feedID, tsTick::SERIALIZEDMAXSIZE);

    if (mysql_query(mCon, sql.GetPtr()))
        throw tsStoreException(strprintf("%s: %s\n", __FUNCTION__, mysql_error(mCon)));
    else
        printf("%s: Created exchange table x%08X%08X\n", __FUNCTION__, (bbU32)(feedID>>32), (bbU32)feedID);
}

enum INSERTPARAM
{
    INSERTPARAM_SYM = 0,
    INSERTPARAM_TT,
    INSERTPARAM_COUNT,
    INSERTPARAM_TIME,
    INSERTPARAM_DATA,
    INSERTPARAMCOUNT
};

tsStoreMySQL::Feed::Feed(tsStoreMySQL& parent, bbU64 feedID) : mFeedID(feedID)
{
    mInsertStmt = mysql_stmt_init(parent.mCon);
    if (!mInsertStmt)
        throw tsStoreException(strprintf("%s: %s\n", __FUNCTION__, mysql_error(parent.mCon)));

    bbStrBuf query;
    query.Printf("INSERT INTO x%08X%08X (sym,tt,count,time,data) VALUES (?,?,?,?,?)", (bbU32)(feedID>>32), (bbU32)feedID);

    if (mysql_stmt_prepare(mInsertStmt, query.GetPtr(), query.GetLen()))
        throw tsStoreException(strprintf("%s: mysql_stmt_prepare() '%s' failed, %d %s\n", __FUNCTION__,
            query.GetPtr(), mysql_errno(parent.mCon), mysql_error(parent.mCon)));

    bbASSERT(mysql_stmt_param_count(mInsertStmt) == INSERTPARAMCOUNT);

    memset(mInsertParam, 0, sizeof(mInsertParam));

    mInsertParam[INSERTPARAM_SYM].buffer_type = MYSQL_TYPE_LONGLONG;
    mInsertParam[INSERTPARAM_SYM].buffer = (char*)&parent.mInsertParam.mSym;
    mInsertParam[INSERTPARAM_SYM].is_unsigned = TRUE;

    mInsertParam[INSERTPARAM_TT].buffer_type = MYSQL_TYPE_SHORT;
    mInsertParam[INSERTPARAM_TT].buffer = (char*)&parent.mInsertParam.mTT;
    mInsertParam[INSERTPARAM_TT].is_unsigned = TRUE;

    mInsertParam[INSERTPARAM_COUNT].buffer_type = MYSQL_TYPE_LONG;
    mInsertParam[INSERTPARAM_COUNT].buffer = (char*)&parent.mInsertParam.mCount;
    mInsertParam[INSERTPARAM_COUNT].is_unsigned = TRUE;

    mInsertParam[INSERTPARAM_TIME].buffer_type = MYSQL_TYPE_LONGLONG;
    mInsertParam[INSERTPARAM_TIME].buffer = (char*)&parent.mInsertParam.mTime;
    mInsertParam[INSERTPARAM_TIME].is_unsigned = TRUE;

    mInsertParam[INSERTPARAM_DATA].buffer_type = MYSQL_TYPE_BLOB;
    mInsertParam[INSERTPARAM_DATA].buffer = parent.mInsertParam.mEscRawTick;
    mInsertParam[INSERTPARAM_DATA].buffer_length = sizeof(parent.mInsertParam.mEscRawTick);
    mInsertParam[INSERTPARAM_DATA].length = &parent.mInsertParam.mEscRawTickLength;

    if (mysql_stmt_bind_param(mInsertStmt, mInsertParam))
        throw tsStoreException(strprintf("%s: mysql_stmt_bind_param() failed, %s\n", __FUNCTION__, mysql_error(parent.mCon)));
}

tsStoreMySQL::Feed::~Feed()
{
    if (mInsertStmt)
        mysql_stmt_close(mInsertStmt);
}

tsStoreMySQL::Feed* tsStoreMySQL::GetFeed(bbU64 feedID)
{
    FeedMap::const_iterator it = mFeedMap.find(feedID);
    if (it == mFeedMap.end())
    {
        CreateFeedTable(feedID);
        Feed* pFeed = new Feed(*this, feedID);
        mFeedMap.insert(std::pair<bbU64, Feed*>(feedID, pFeed));
        return pFeed;
    }
    return it->second;
}

void tsStoreMySQL::InsertTick(tsStoreMySQL::Feed* pFeed, tsTick& tick, const char* pRawTick, bbUINT tickSize)
{
    mInsertParam.mSym   = tick.objID().symbolID();
    mInsertParam.mTT    = tick.type();
    mInsertParam.mCount = tick.count();
    mInsertParam.mTime  = tick.time();
    mInsertParam.mEscRawTickLength = mysql_real_escape_string(mCon, mInsertParam.mEscRawTick, pRawTick, tickSize);

    if (mysql_stmt_execute(pFeed->mInsertStmt))
        throw tsStoreException(strprintf("%s: mysql_stmt_execute() failed, %s\n", __FUNCTION__, mysql_error(mCon)));
}

void tsStoreMySQL::SaveTick(const char* pRawTick, bbUINT tickSize)
{
    bbASSERT(tickSize <= tsTick::SERIALIZEDMAXSIZE);

    tsTickUnion tickUnion;
    tsTick& tick = tickUnion;
    tick.unserializeHead(pRawTick);

    tsMutexLocker lock(mMutex);

    Feed* pFeed = GetFeed(tick.mObjID.feedID());
    InsertTick(pFeed, tick, pRawTick, tickSize);
}


tsStoreMySQL::Query::Query(MYSQL* pCon, const char* sql) : mCon(pCon), mResult(NULL)
{
    if (sql)
        Exec(sql);
}

void tsStoreMySQL::Query::Clear()
{
    if (mResult)
        mysql_free_result(mResult);
    mResult = NULL;
}

void tsStoreMySQL::Query::Exec(const char* sql)
{
    Clear();

    if (mysql_query(mCon, sql))
        throw tsStoreException(strprintf("%s: %s\n", __FUNCTION__, mysql_error(mCon)));

    mResult = mysql_store_result(mCon);
    if (!mResult)
        throw tsStoreException(strprintf("%s: %s\n", __FUNCTION__, mysql_error(mCon)));
}

MYSQL_ROW tsStoreMySQL::Query::FetchRow()
{
    MYSQL_ROW row = mysql_fetch_row(mResult);
    mLengths = mysql_fetch_lengths(mResult);
    return row;
}

MYSQL_ROW tsStoreMySQL::Query::ExecAndFetchRow(const char* sql)
{
    Exec(sql);
    return FetchRow();
}

bbU64 tsStoreMySQL::Authenticate(bbU64 uid, const bbU8* pPwd)
{
    tsMutexLocker lock(mMutex);

    bbStrBuf sql;
    sql.Printf("SELECT pwd,salt FROM user WHERE id=%"bbI64"u", uid);
    Query q(mCon, sql.GetPtr());

    // - Check if UID exists
    MYSQL_ROW row = q.FetchRow();
    if (!row)
    {
        printf("%s: unknown UID 0x%"bbI64"X\n", __FUNCTION__, uid);
        return 0;
    }

    if (!row[0] || !row[1] || q.GetFieldLen(0)!=32 || q.GetFieldLen(1)!=32)
        throw tsStoreException(strprintf("%s: unexpected user query result\n", __FUNCTION__));

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
        throw tsStoreException(e.what());
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

bbU64 tsStoreMySQL::CreateUser(std::string name, const bbU8* pPwd)
{
    tsMutexLocker lock(mMutex);

    bbStrBuf sql("INSERT INTO user (name, pwd, salt) values (\"");

    bbStrBuf escaped;
    if (!escaped.SetLen(name.size()*2))
        throw tsStoreException(strprintf("%s: out of memory\n", __FUNCTION__));
    mysql_escape_string(escaped.GetPtr(), name.c_str(), name.size());

    sql += escaped.GetPtr();
    sql += "\",\"";

    if (!escaped.SetLen(32*2))
        throw tsStoreException(strprintf("%s: out of memory\n", __FUNCTION__));

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
        throw tsStoreException(strprintf("%s: %s\n", __FUNCTION__, mysql_error(mCon)));

    bbU64 uid = mysql_insert_id(mCon);
    printf("%s: Created user %s with uid 0x%"bbI64"X\n", __FUNCTION__, name.c_str(), uid);
    return uid;
}

