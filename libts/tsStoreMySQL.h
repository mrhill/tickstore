#ifndef tsSTOREMYSQL_H
#define tsSTOREMYSQL_H

#include "tsStore.h"

#include <my_global.h>
#include <mysql.h>

class tsStoreMySQL : public tsStore
{
    MYSQL *mCon;

    void CreateExchange(bbU32 exchangeID);

public:
    tsStoreMySQL(tsTickFactory& tickFactory, const char* pDBName);
    virtual ~tsStoreMySQL();
    virtual void SaveTick(const char* pRawTick, bbUINT tickSize);
};

#endif

