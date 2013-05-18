#ifndef tsSTOREMYSQL_H
#define tsSTOREMYSQL_H

#include "tsStore.h"
#include "tsMutex.h"
#include <map>
#include <my_global.h>
#include <mysql.h>

class tsStoreMySQL : public tsStore
{
    MYSQL* mCon;
    tsMutex mMutex;

    struct InsertParam
    {
        bbU64 mSym;
        bbU64 mTime;
        unsigned long mEscRawTickLength;
        bbU32 mCount;
        bbU16 mTT;
        char  mEscRawTick[tsTick::SERIALIZEDMAXSIZE*2 + 1];
    };

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
    InsertParam mInsertParam;

    void CreateExchangeTable(bbU32 exchangeID);
    Exchange* GetExchange(bbU32 exchangeID);
    void InsertTick(Exchange* pExchange, tsTick& tick, const char* pRawTick, bbUINT tickSize);

public:
    tsStoreMySQL(tsTickFactory& tickFactory, const char* pDBName);
    virtual ~tsStoreMySQL();
    virtual void SaveTick(const char* pRawTick, bbUINT tickSize);
};

#endif

