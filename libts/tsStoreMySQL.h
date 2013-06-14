#ifndef tsSTOREMYSQL_H
#define tsSTOREMYSQL_H

#include "tsStore.h"
#include "tsMutex.h"
#include "tsMySQL.h"
#include <map>

class tsStoreMySQL : public tsStore
{
    tsMySQLCon mCon;
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

    void CreateFeedTable(bbU64 feedID);
    Feed* GetFeed(bbU64 feedID);
    void InsertTick(Feed* pFeed, tsTick& tick, const char* pRawTick, bbUINT tickSize);

public:
    tsStoreMySQL(const char* pDBName);
    virtual ~tsStoreMySQL();
    virtual void SaveTick(const char* pRawTick, bbUINT tickSize);
};

#endif

