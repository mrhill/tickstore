#ifndef tsNode_H
#define tsNode_H

#include "tsdef.h"
#include "tsMutex.h"
#include "tsSocket.h"
#include "tsSession.h"
#include "tsTracker.h"
#include "tsTickReceiver.h"
#include <map>

class tsTickFactory;
class tsStore;

class tsNode : public tsThread, protected tsTickReceiver
{
    tsSocket        mInPipe;
    tsMutex         mNodeMutex;
    tsTickFactory&  mFactory;
    tsTracker&      mTracker;
    tsStore&        mStore;
    int             mNextSessionID;

    typedef std::multimap<bbU64, tsSession*> SubscriberMap;
    SubscriberMap mSubscriberMap;

    static void cancelSession(tsSession* s);
    static void joinSession(tsSession* s);
    tsVecManagedPtr<tsSession> mSessions;
    tsVecManagedPtr<tsSession> mInactiveSessions;

    virtual void* run();
    virtual void Proc(const char* pRawTick, bbUINT tickSize);

    friend class tsSession;
    void CreateSession(int socketFD);
protected:
    void DeactivateSession(tsSession* pSession);

public:
    tsNode(tsTickFactory& factory, tsTracker& tracker, tsStore& store);
    ~tsNode();

    void SubscribeFeed(bbU64 feedID, tsSession* pSession);
};

#endif

