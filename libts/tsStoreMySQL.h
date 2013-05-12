#ifndef tsSTOREMYSQL_H
#define tsSTOREMYSQL_H

#include "tsStore.h"
#include <map>
#include <my_global.h>
#include <mysql.h>

class tsStoreMySQL : public tsStore
{
    MYSQL* mCon;

    struct Exchange
    {
        bbU32       mExchangeID;
        MYSQL_STMT* mInsertStmt;
        MYSQL_BIND  mInsertParam[5];

        Exchange(tsStoreMySQL& parent, bbU32 exchangeID);
        ~Exchange();
    };

    typedef std::map<bbU32, Exchange*> ExchangeMap;
    ExchangeMap mExchangeMap;

    bbU64 mParamSym;
    bbU16 mParamTT;
    bbU32 mParamCount;
    bbU64 mParamTime;
    char mParamEscRawTick[tsTick::SERIALIZEDMAXSIZE*2 + 1];

    void CreateExchangeTable(bbU32 exchangeID);
    Exchange* GetExchange(bbU32 exchangeID);
    void InsertTick(Exchange* pExchange, tsTick& tick, const char* pRawTick, bbUINT tickSize);

public:
    tsStoreMySQL(tsTickFactory& tickFactory, const char* pDBName);
    virtual ~tsStoreMySQL();
    virtual void SaveTick(const char* pRawTick, bbUINT tickSize);
};

#endif

