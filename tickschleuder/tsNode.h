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

    void ProcessControlMsg(const char* pMsg, bbUINT msgSize, zmq::message_t* pMsgMore);
    void DestroySession(tsSession* pSession);

public:
    tsNode(zmq::context_t& zmq, tsTracker& tracker);
    ~tsNode();
    void* run();

    virtual void ProcessTick(const char* pRawTick, bbUINT tickSize);

    void Authenticate(int sessionID, const char* pRawAuthTick, bbUINT tickSize);

    void SubscribeFeed(bbU64 feedID, bbU64 queryID, tsSession* pSession);
    void UnsubscribeAllFeeds(tsSession* pSession);
};

#endif

