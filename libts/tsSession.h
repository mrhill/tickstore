#ifndef tsSession_H
#define tsSession_H

#include "tsdef.h"
#include "tsStore.h"
#include "tsThread.h"
#include "tsTickReceiver.h"
#include <set>

class tsNode;

class tsSession : protected tsThread, protected tsTickReceiver, protected tsTickListener
{
    friend class tsNode;

    tsNode&         mNode;
    tsStore&        mStore;

    struct FeedFilter
    {
        bbU64 mLastFeedIDCache;
        bbU8  mAllowAll;
        std::set<bbU64>* mHashTbl[256];

        FeedFilter()
        {
            mLastFeedIDCache = 0;
            mAllowAll = 0;
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

        inline bool isAllowed(bbU64 feedID) const
        {
            std::set<bbU64>* pTbl;
            return mLastFeedIDCache == feedID ||
                   ((pTbl = mHashTbl[(bbUINT)feedID & 255]),
                    (pTbl && pTbl->find(feedID) != pTbl->end())) ||
                   mAllowAll;
        }
    };

    FeedFilter mInFilter;
    int mSessionID;

    void SubscribeFeed(bbU64 feedID);

    virtual void* run();
    virtual void ProcessTick(const char* pRawTick, bbUINT tickSize);

protected:
    void SendOut(const char* pRawTick, bbUINT tickSize);

public:
    tsSession(tsTickFactory& tickFactory, tsNode& node, tsStore& store, int fd, int procID);
    ~tsSession();
};

#endif

