#include "tsStoreMySQL.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <babel/StrBuf.h>

tsStoreMySQL::tsStoreMySQL(const char* pDBName)
  : mCon(pDBName)
{
}

tsStoreMySQL::~tsStoreMySQL()
{
    for (FeedMap::iterator it = mFeedMap.begin(); it != mFeedMap.end(); it++)
        delete it->second;
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

