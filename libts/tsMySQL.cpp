#include "tsMySQL.h"
#include <stdio.h>

tsMySQLCon::tsMySQLCon(const char* dbname, const char* host, bbU16 port, const char* user, const char* pass)
{
    printf("%s: MySQL client version: %s\n", __FUNCTION__, mysql_get_client_info());

    mCon = mysql_init(NULL);
    if (!mCon)
        throw tsMySQLException(strprintf("%s: Error on mysql_init()\n", __FUNCTION__));

    if (!mysql_real_connect(mCon, host ? host: "localhost", user, pass, NULL, port, NULL, 0))
        throw tsMySQLException(strprintf("%s: %s\n", __FUNCTION__, mysql_error(mCon)));

    printf("%s: MySQL host: %s\n", __FUNCTION__, mysql_get_host_info(mCon));

    if (mysql_query(mCon, strprintf("CREATE DATABASE IF NOT EXISTS %s", dbname).c_str()) ||
        mysql_select_db(mCon, dbname))
        throw tsMySQLException(strprintf("%s: %s\n", __FUNCTION__, mysql_error(mCon)));
    else
        printf("Selected database %s\n", dbname);
}

tsMySQLCon::~tsMySQLCon()
{
    if (mCon)
    {
        printf("%s: Closing MySQL connection\n", __FUNCTION__);
        mysql_close(mCon);
    }
}

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

