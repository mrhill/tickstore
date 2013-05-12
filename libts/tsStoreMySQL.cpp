#include "tsStoreMySQL.h"
#include <stdio.h>
#include <stdexcept>
#include <babel/StrBuf.h>

tsStoreMySQL::tsStoreMySQL(tsTickFactory& tickFactory, const char* pDBName)
  : tsStore(tickFactory), mCon(NULL)
{
    printf("%s: MySQL client version: %s\n", __FUNCTION__, mysql_get_client_info());

    mCon = mysql_init(NULL);
    if (!mCon)
        throw std::runtime_error(strprintf("%s: Error on mysql_init()\n", __FUNCTION__));

    if (!mysql_real_connect(mCon, "localhost", "", "", NULL, 0, NULL, 0))
        throw std::runtime_error(strprintf("%s: %s\n", __FUNCTION__, mysql_error(mCon)));

    printf("%s: MySQL host: %s\n", __FUNCTION__, mysql_get_host_info(mCon));

    if (mysql_query(mCon, strprintf("CREATE DATABASE IF NOT EXISTS %s", pDBName).c_str()) ||
        mysql_select_db(mCon, pDBName))
        throw std::runtime_error(strprintf("%s: %s\n", __FUNCTION__, mysql_error(mCon)));
    else
        printf("Selected database %s\n", pDBName);
}

tsStoreMySQL::~tsStoreMySQL()
{
    for (ExchangeMap::iterator it = mExchangeMap.begin(); it != mExchangeMap.end(); it++)
        delete it->second;

    printf("%s: Closing MySQL connection\n", __FUNCTION__);
    mysql_close(mCon);
}

void tsStoreMySQL::CreateExchangeTable(bbU32 exchangeID)
{
    char tableName[12];
    snprintf(tableName, 12, "x%08X", exchangeID);

    bbStrBuf sql;
    sql.Printf("CREATE TABLE IF NOT EXISTS %s ("
                   "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE KEY,"
                   "sym BIGINT UNSIGNED NOT NULL DEFAULT 0,"
                   "tt SMALLINT UNSIGNED NOT NULL DEFAULT 0,"
                   "count INT UNSIGNED NOT NULL DEFAULT 0,"
                   "time BIGINT UNSIGNED NOT NULL DEFAULT 0,"
                   "data VARBINARY(%u) NOT NULL DEFAULT '')", tableName, tsTick::SERIALIZEDMAXSIZE);

    if (mysql_query(mCon, sql.GetPtr()))
        throw std::runtime_error(strprintf("%s: %s\n", __FUNCTION__, mysql_error(mCon)));
    else
        printf("%s: Created exchange table %s\n", __FUNCTION__, tableName);
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

tsStoreMySQL::Exchange::Exchange(tsStoreMySQL& parent, bbU32 exchangeID) : mExchangeID(exchangeID)
{
    mInsertStmt = mysql_stmt_init(parent.mCon);
    if (!mInsertStmt)
        throw std::runtime_error(strprintf("%s: %s\n", __FUNCTION__, mysql_error(parent.mCon)));

    bbStrBuf query;
    query.Printf("INSERT INTO x%08X (sym,tt,count,time,data) VALUES (?,?,?,?,\"?\")", exchangeID);

    if (mysql_stmt_prepare(mInsertStmt, query.GetPtr(), query.GetLen()))
        throw std::runtime_error(strprintf("%s: mysql_stmt_prepare() '%s' failed, %d %s\n", __FUNCTION__,
            query.GetPtr(), mysql_errno(parent.mCon), mysql_error(parent.mCon)));

    bbASSERT(mysql_stmt_param_count(mInsertStmt) == INSERTPARAMCOUNT);

    memset(mInsertParam, 0, sizeof(mInsertParam));

    mInsertParam[INSERTPARAM_SYM].buffer_type = MYSQL_TYPE_LONGLONG;
    mInsertParam[INSERTPARAM_SYM].buffer = (char*)&parent.mParamSym;
    mInsertParam[INSERTPARAM_SYM].is_unsigned = TRUE;

    mInsertParam[INSERTPARAM_TT].buffer_type = MYSQL_TYPE_SHORT;
    mInsertParam[INSERTPARAM_TT].buffer = (char*)&parent.mParamTT;
    mInsertParam[INSERTPARAM_TT].is_unsigned = TRUE;

    mInsertParam[INSERTPARAM_COUNT].buffer_type = MYSQL_TYPE_LONG;
    mInsertParam[INSERTPARAM_COUNT].buffer = (char*)&parent.mParamCount;
    mInsertParam[INSERTPARAM_COUNT].is_unsigned = TRUE;

    mInsertParam[INSERTPARAM_TIME].buffer_type = MYSQL_TYPE_LONGLONG;
    mInsertParam[INSERTPARAM_TIME].buffer = (char*)&parent.mParamTime;
    mInsertParam[INSERTPARAM_TIME].is_unsigned = TRUE;

    mInsertParam[INSERTPARAM_DATA].buffer_type = MYSQL_TYPE_BLOB;
    mInsertParam[INSERTPARAM_DATA].buffer = parent.mParamEscRawTick;
    mInsertParam[INSERTPARAM_DATA].buffer_length = tsTick::SERIALIZEDMAXSIZE;
    mInsertParam[INSERTPARAM_DATA].length = &mInsertParam[INSERTPARAM_DATA].buffer_length;

    if (mysql_stmt_bind_param(mInsertStmt, mInsertParam))
        throw std::runtime_error(strprintf("%s: mysql_stmt_bind_param() failed, %s\n", __FUNCTION__, mysql_error(parent.mCon)));
}

tsStoreMySQL::Exchange::~Exchange()
{
    if (mInsertStmt)
        mysql_stmt_close(mInsertStmt);
}

void tsStoreMySQL::InsertTick(tsStoreMySQL::Exchange* pExchange, tsTick& tick, const char* pRawTick, bbUINT tickSize)
{
    //xxx
    mParamSym   = tick.objID().symbolID();
    mParamTT    = tick.type();
    mParamCount = tick.count();
    mParamTime  = tick.time();
    mysql_real_escape_string(mCon, mParamEscRawTick, pRawTick, tickSize);

    if (mysql_stmt_execute(pExchange->mInsertStmt))
        throw std::runtime_error(strprintf("%s: mysql_stmt_execute() failed, %s\n", __FUNCTION__, mysql_error(mCon)));
}

tsStoreMySQL::Exchange* tsStoreMySQL::GetExchange(bbU32 exchangeID)
{
    ExchangeMap::const_iterator it = mExchangeMap.find(exchangeID);
    if (it == mExchangeMap.end())
    {
        CreateExchangeTable(exchangeID);
        Exchange* pExchange = new Exchange(*this, exchangeID);
        mExchangeMap.insert(std::pair<bbU32, Exchange*>(exchangeID, pExchange));
        return pExchange;
    }
    return it->second;
}

void tsStoreMySQL::SaveTick(const char* pRawTick, bbUINT tickSize)
{
    bbASSERT(tickSize <= tsTick::SERIALIZEDMAXSIZE);

    tsTickUnion tickUnion;
    char sql[64 + 36+2*22+2*12 + tsTick::SERIALIZEDMAXSIZE*2 + 1];

    tsTick& tick = tickUnion;
    tick.unserializeHead(pRawTick);

    Exchange* pExchange = GetExchange(tick.mObjID.exchangeID());
    InsertTick(pExchange, tick, pRawTick, tickSize);

    /*
    char* pBuf = sql + snprintf(sql, sizeof(sql) - tsTick::SERIALIZEDMAXSIZE*2,
                                "INSERT INTO x%08X (sym,tt,count,time,data) VALUES (%"bbI64"u,%u,%u,%"bbI64"u,\"",
                                tick.mObjID.exchangeID(), tick.mObjID.symbolID(), (bbUINT)tick.type(), tick.count(), tick.time());
    pBuf += mysql_real_escape_string(mCon, pBuf, pRawTick, tickSize);
    pBuf[0] = '"';
    pBuf[1] = ')';
    pBuf[2] = 0;

    puts(sql);
    if (mysql_query(mCon, sql))
        throw std::runtime_error(strprintf("%s: %s\n", __FUNCTION__, mysql_error(mCon)));
    */
}

