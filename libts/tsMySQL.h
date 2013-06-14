#ifndef tsMySQL_H_
#define tsMySQL_H_

#include "tsdef.h"
#include <stdexcept>
#include <my_global.h>
#include <mysql.h>

class tsMySQLException : public std::runtime_error
{
public:
    tsMySQLException(const std::string& what_arg) : std::runtime_error(what_arg) {}
};

class tsMySQLCon
{
    MYSQL* mCon;
public:
    tsMySQLCon(const char* dbname, const char* host = NULL, bbU16 port = 0, const char* user="", const char* pass="");
    ~tsMySQLCon();
    inline operator MYSQL*() const { return mCon; }
};

class tsMySQLQuery
{
    MYSQL* mCon;
    MYSQL_RES* mResult;
    unsigned long* mLengths;
public:
    tsMySQLQuery(MYSQL* pCon, const char* sql = NULL);
    ~tsMySQLQuery() { Clear(); }
    void Exec(const char* sql);
    void Clear();
    MYSQL_ROW FetchRow();
    MYSQL_ROW ExecAndFetchRow(const char* sql);
    inline unsigned long GetFieldLen(bbUINT i) const { return mLengths[i]; }
};

#endif

