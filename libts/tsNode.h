#ifndef tsNode_H
#define tsNode_H

#include "tsdef.h"
#include "tsSocket.h"
#include "tsSession.h"
#include "tsTracker.h"
#include "tsTickReceiver.h"
#include <map>
#include <list>
#include <zmq.hpp>

class tsNode : public tsTickListener
{
    tsSocket         mClientListen;
    tsSocket         mPipeListen;

    std::list<tsTickReceiver*> mPipeTCPConnections;
    tsVecManagedPtr<tsSession> mSessions;

    zmq::socket_t    mAuthSocket;
    int              mAuthSocketFD;

    zmq::context_t&  mZmq;
    tsTracker&       mTracker;
    int              mNextSessionID;

    static const unsigned MaxClientConnections = tsSocketSet::MAXSETSIZE/2;
    static const unsigned MaxPipeConnections = tsSocketSet::MAXSETSIZE/2 - 10;

    typedef std::pair<bbU64, bbU64> SubscriberKey;
    typedef std::multimap<SubscriberKey, tsSession*> SubscriberMap;
    SubscriberMap mSubscriberMap;

    void ProcessAuthReply(zmq::message_t& msg);
    void DestroySession(tsSession* pSession);

public:
    tsNode(zmq::context_t& zmq, tsTracker& tracker);
    ~tsNode();
    void* run();

    virtual void ProcessTick(const char* pRawTick, bbUINT tickSize);

    void Authenticate(int sessionID, bbU64 uid, const bbU8* pPwd);

    void SubscribeFeed(bbU64 feedID, bbU64 queryID, tsSession* pSession);
    void UnsubscribeAllFeeds(tsSession* pSession);
};

#endif

