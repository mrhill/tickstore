#include "tsMySQL.h"

tsMySQLQuery::tsMySQLQuery(MYSQL* pCon, const char* sql) : mCon(pCon), mResult(NULL)
{
    if (sql)
        Exec(sql);
}

void tsMySQLQuery::Clear()
{
    if (mResult)
        mysql_free_result(mResult);
    mResult = NULL;
}

void tsMySQLQuery::Exec(const char* sql)
{
    Clear();

    if (mysql_query(mCon, sql))
        throw tsMySQLException(strprintf("%s: %s\n", __FUNCTION__, mysql_error(mCon)));

    mResult = mysql_store_result(mCon);
    if (!mResult)
        throw tsMySQLException(strprintf("%s: %s\n", __FUNCTION__, mysql_error(mCon)));
}

MYSQL_ROW tsMySQLQuery::FetchRow()
{
    MYSQL_ROW row = mysql_fetch_row(mResult);
    mLengths = mysql_fetch_lengths(mResult);
    return row;
}

MYSQL_ROW tsMySQLQuery::ExecAndFetchRow(const char* sql)
{
    Exec(sql);
    return FetchRow();
}

