#ifndef tsTICKPROCSCHLEUDER_H
#define tsTICKPROCSCHLEUDER_H

#include "tsTickReceiver.h"
#include "tsStore.h"
#include "tsMon.h"
#include "tsObjID.h"
#include <string.h>
#include <map>
#include <set>
#include <vector>

class tsTickProcSchleuder : public tsTickReceiver
{
    tsStore&    mStore;
    bbUINT      mGroup;

    struct FeedFilter
    {
        std::set<bbU64>* mHashTbl[256];

        FeedFilter()
        {
            memset(mHashTbl, 0, sizeof(mHashTbl));
        }

        ~FeedFilter()
        {
            for(int i=bbARRSIZE(mHashTbl)-1; i>=0; i--)
                delete mHashTbl[i];
        }

        void AddFeed(bbU64 feedID)
        {
            bbUINT i = (bbUINT)feedID & 255;
            if (!mHashTbl[i])
                mHashTbl[i] = new std::set<bbU64>();
            mHashTbl[i]->insert(feedID);
        }

        inline bool isAllowed(bbU64 feedID)
        {
            std::set<bbU64>* pTbl = mHashTbl[(bbUINT)feedID & 255];
            return pTbl && pTbl->find(feedID) != pTbl->end();
        }
    };

    FeedFilter mInFilter;

    typedef std::map<tsObjID, tsMonSymbol*> ObjIDMonMap;
    ObjIDMonMap mObjID2Mon;
public:
    tsTickProcSchleuder(tsTickFactory& tickFactory, tsStore& store, int socketFD, int procID, bbUINT group);

    tsMonSymbol* GetMon(const tsObjID& objID);

    virtual void Proc(const char* pRawTick, bbUINT tickSize);
    virtual void Proc_(const tsTick& tick);
};

#endif

