#ifndef tsNode_H
#define tsNode_H

#include "tsdef.h"
#include "tsSocket.h"
#include "tsSession.h"
#include "tsTracker.h"
#include "tsTickReceiver.h"
#include <map>
#include <list>

class tsTickFactory;
class tsStore;

class tsNode : public tsTickListener
{
    tsSocket         mClientListen;
    tsSocket         mPipeListen;

    std::list<tsTickReceiver*> mPipeTCPConnections;
    tsVecManagedPtr<tsSession> mSessions;

    tsTickFactory&   mFactory;
    tsTracker&       mTracker;
    tsStore&         mStore;
    int              mNextSessionID;

    static const unsigned MaxClientConnections = tsSocketSet::MAXSETSIZE/2;
    static const unsigned MaxPipeConnections = tsSocketSet::MAXSETSIZE/2 - 10;

    typedef std::multimap<bbU64, tsSession*> SubscriberMap;
    SubscriberMap mSubscriberMap;


public:
    tsNode(tsTickFactory& factory, tsTracker& tracker, tsStore& store);
    ~tsNode();
    void* run();

    virtual void ProcessTick(const char* pRawTick, bbUINT tickSize);

    void SubscribeFeed(bbU64 feedID, tsSession* pSession);
    void UnsubscribeAllFeeds(tsSession* pSession);
};

#endif

