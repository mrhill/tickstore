#ifndef tsSession_H
#define tsSession_H

#include "tsdef.h"
#include "tsTickReceiver.h"
#include "tsAuth.h"
#include <set>

class tsNode;

class tsSession : public tsTickReceiver, protected tsTickListener
{
    tsNode& mNode;
    tsUser  mUser;
    bbU32   mTickSendCount;

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

    virtual void ProcessTick(const char* pRawTick, bbUINT tickSize);

public:
    tsSession(tsNode& node, int fd, int procID);
    ~tsSession();
    void close();
    const tsUser& user() const { return mUser; }

    void SendTick(tsTick& tick);
    void SendTick(const char* pRawTick, bbUINT tickSize);
    void SetUser(const bbU8* pUser, bbUINT bufSize);
    static int cmpSessionID(const void *p1, const void *p2);
};

#endif

