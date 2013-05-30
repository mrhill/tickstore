#ifndef tsNode_H
#define tsNode_H

#include "tsdef.h"
#include "tsSocket.h"
#include "tsSession.h"
#include "tsTracker.h"
#include "tsTickReceiver.h"
#include <map>
#include <list>

class tsStore;

class tsNode : public tsTickListener
{
    tsSocket         mClientListen;
    tsSocket         mPipeListen;

    std::list<tsTickReceiver*> mPipeTCPConnections;
    tsVecManagedPtr<tsSession> mSessions;

    tsTracker&       mTracker;
    tsStore&         mStore;
    int              mNextSessionID;

    static const unsigned MaxClientConnections = tsSocketSet::MAXSETSIZE/2;
    static const unsigned MaxPipeConnections = tsSocketSet::MAXSETSIZE/2 - 10;

    typedef std::pair<bbU64, bbU64> SubscriberKey;
    typedef std::multimap<SubscriberKey, tsSession*> SubscriberMap;
    SubscriberMap mSubscriberMap;


public:
    tsNode(tsTracker& tracker, tsStore& store);
    ~tsNode();
    void* run();

    virtual void ProcessTick(const char* pRawTick, bbUINT tickSize);

    void SubscribeFeed(bbU64 feedID, bbU64 queryID, tsSession* pSession);
    void UnsubscribeAllFeeds(tsSession* pSession);
};

#endif

