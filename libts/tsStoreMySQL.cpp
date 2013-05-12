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

    CreateExchange(0x42);
}

tsStoreMySQL::~tsStoreMySQL()
{
    printf("%s: Closing MySQL connection\n", __FUNCTION__);
    mysql_close(mCon);
}

void tsStoreMySQL::CreateExchange(bbU32 exchangeID)
{
    char tableName[12];
    snprintf(tableName, 12, "x%08X", exchangeID);

    bbStrBuf sql;
    sql.Printf("CREATE TABLE IF NOT EXISTS %s ("
                   "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE KEY,"
                   "objid BIGINT UNSIGNED NOT NULL DEFAULT 0,"
                   "tt SMALLINT UNSIGNED NOT NULL DEFAULT 0,"
                   "count INT UNSIGNED NOT NULL DEFAULT 0,"
                   "time BIGINT UNSIGNED NOT NULL DEFAULT 0,"
                   "data VARBINARY(%u) NOT NULL DEFAULT '')", tableName, tsTick::SERIALIZEDMAXSIZE);

    if (mysql_query(mCon, sql.GetPtr()))
        throw std::runtime_error(strprintf("%s: %s\n", __FUNCTION__, mysql_error(mCon)));
    else
        printf("%s: Created exchange table %s\n", __FUNCTION__, tableName);
}

void tsStoreMySQL::SaveTick(const char* pRawTick, bbUINT tickSize)
{
    tsTickUnion tickUnion;
    tsTick& tick = tickUnion;
    tick.unserializeHead(pRawTick);

}

