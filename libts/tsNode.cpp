#include "tsNode.h"
#include <stdexcept>
#include <iostream>
#include <algorithm>

tsNode::tsNode(tsTickFactory& factory, tsTracker& tracker, tsStore& store)
  : tsTickReceiver(factory),
    mFactory(factory),
    mTracker(tracker),
    mStore(store),
    mInPipe(tsSocketType_UDP),
    mNextSessionID(0)
{
    mInPipe.bind(NULL, 2228);
    std::cout << __FUNCTION__ << ": in pipe listening on UDP " << mInPipe.nameinfo() << std::endl;

    start();
}

tsNode::~tsNode()
{
    // first shut down server thread so no new sessions are created
    cancel();
    join();

    // shut down sessions
    std::for_each(mSessions.begin(), mSessions.end(), tsNode::cancelSession);
    std::for_each(mSessions.begin(), mSessions.end(), tsNode::joinSession);
    mSessions.clear();

    mInactiveSessions.clear();
}

void tsNode::cancelSession(tsSession* s) { s->cancel(); }
void tsNode::joinSession(tsSession* s) { s->join(); }

void tsNode::CreateSession(int socketFD)
{
    tsMutexLocker lock(mNodeMutex);

    mInactiveSessions.clear(); // todo: move this to a signal handler

    tsSession* pSession = new tsSession(mFactory, *this, mStore, socketFD, mNextSessionID++);
    mSessions.push_back(pSession);
}

void tsNode::DeactivateSession(tsSession* pSession)
{
    tsMutexLocker lock(mNodeMutex);

    // unsubscribe all feeds for this session
    for (SubscriberMap::iterator it=mSubscriberMap.begin(); it!=mSubscriberMap.end(); )
    {
        SubscriberMap::iterator it_inc = it; it_inc++;
        if (it->second == pSession)
            mSubscriberMap.erase(it);
        it = it_inc;
    }

    // move session to inactive session list
    std::vector<tsSession*>::iterator it = std::find(mSessions.begin(), mSessions.end(), pSession);
    if (it != mSessions.end())
    {
        mSessions.erase(it);
        mInactiveSessions.push_back(pSession);
    }
}

void* tsNode::run()
{
    try
    {
        int listenPort = 2227;
        std::cout << __FUNCTION__ << ": listening for connections on port " << listenPort << ' ' << std::endl;
        tsSocket listenSocket(tsSocketType_TCP);
        listenSocket.listen(listenPort);

        while (!testCancel())
        {
            tsSocketSet socketSet;
            socketSet.addRdFD(listenSocket.fd());
            socketSet.addRdFD(mInPipe.fd());
            if (socketSet.select())
            {
                try
                {
                    if (socketSet.testRdFD(mInPipe.fd()))
                    {
                        tsTickReceiver::receive(mInPipe, 0);
                    }

                    if (socketSet.testRdFD(listenSocket.fd()))
                    {
                        int newSocketOut = listenSocket.accept();
                        printf("%s: new connection on port %d with fd %d\n", __FUNCTION__, listenPort, newSocketOut);
                        CreateSession(newSocketOut);
                    }
                }
                catch(tsSocketException& e)
                {
                    printf("%s: %s\n", __FUNCTION__, e.what());
                }
            }
        }
    }
    catch(std::exception& e)
    {
        std::cout << e.what();
    }

    printf("%s: shutting down\n", __FUNCTION__);
    return NULL;
}

void tsNode::SubscribeFeed(bbU64 feedID, tsSession* pSession)
{
    tsMutexLocker lock(mNodeMutex);

    if (mSubscriberMap.find(feedID) != mSubscriberMap.end()) // node already subscribing to feedID?
    {
        // return if this session is already registered for this feedID
        std::pair<SubscriberMap::const_iterator,SubscriberMap::const_iterator> range = mSubscriberMap.equal_range(feedID);
        for (SubscriberMap::const_iterator it=range.first; it!=range.second; ++it)
            if (it->second == pSession)
                return;
    }
    else
    {
        // subscribe node to feedID
        std::string node = mInPipe.nameinfo();
        std::cout << __FUNCTION__ << ": subscribing node for feedID " << feedID << std::endl;
        if (node.size())
            mTracker.Subscribe(node, feedID);
    }

    // map feedID to session
    mSubscriberMap.insert(std::pair<bbU64, tsSession*>(feedID, pSession));
}

void tsNode::Proc(const char* pRawTick, bbUINT tickSize)
{
    bbU64 feedID = tsTick::unserializeHead_peekFeedID(pRawTick);
    std::pair<SubscriberMap::const_iterator,SubscriberMap::const_iterator> range = mSubscriberMap.equal_range(feedID);
    for (SubscriberMap::const_iterator it=range.first; it!=range.second; ++it)
        it->second->SendOut(pRawTick, tickSize);
}

