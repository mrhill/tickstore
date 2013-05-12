#include "tsStoreMySQL.h"
#include <stdio.h>
#include <stdexcept>

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
    printf("%s: Closing MySQL connection\n", __FUNCTION__);
    mysql_close(mCon);
}

void tsStoreMySQL::SaveTick(const char* pRawTick, bbUINT tickSize)
{
}

