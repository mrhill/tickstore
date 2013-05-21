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

    struct Query
    {
        MYSQL* mCon;
        MYSQL_RES* mResult;
        unsigned long* mLengths;

        Query(MYSQL* pCon, const char* sql = NULL);
        ~Query() { Clear(); }
        void Exec(const char* sql);
        void Clear();
        MYSQL_ROW FetchRow();
        MYSQL_ROW ExecAndFetchRow(const char* sql);
        inline unsigned long GetFieldLen(bbUINT i) const { return mLengths[i]; }
    };

    struct InsertParam
    {
        bbU64 mSym;
        bbU64 mTime;
        unsigned long mEscRawTickLength;
        bbU32 mCount;
        bbU16 mTT;
        char  mEscRawTick[tsTick::SERIALIZEDMAXSIZE*2 + 1];
    };

    struct Feed
    {
        bbU64       mFeedID;
        MYSQL_STMT* mInsertStmt;
        MYSQL_BIND  mInsertParam[5];

        Feed(tsStoreMySQL& parent, bbU64 feedID);
        ~Feed();
    };

    typedef std::map<bbU64, Feed*> FeedMap;
    FeedMap mFeedMap;
    InsertParam mInsertParam;

    void CreateUserTable();
    void CreateFeedTable(bbU64 feedID);
    Feed* GetFeed(bbU64 feedID);
    void InsertTick(Feed* pFeed, tsTick& tick, const char* pRawTick, bbUINT tickSize);

public:
    tsStoreMySQL(tsTickFactory& tickFactory, const char* pDBName);
    virtual ~tsStoreMySQL();
    virtual void SaveTick(const char* pRawTick, bbUINT tickSize);
    virtual bbU64 Authenticate(bbU64 uid, const bbU8* pPwd);
};

#endif

