#include "tsNode.h"
#include <stdexcept>
#include <iostream>
#include <algorithm>

tsNode::tsNode(tsTickFactory& factory, tsTracker& tracker, tsStore& store)
  : tsTickReceiver(factory),
    mFactory(factory),
    mTracker(tracker),
    mStore(store),
    mClientListen(tsSocketType_TCP),
    mPipeUDP(tsSocketType_UDP),
    mPipeListen(tsSocketType_TCP),
    mNextSessionID(0)
{
    mClientListen.listen(2227);
    std::cout << __FUNCTION__ << ": listening on TCP " << mClientListen.nameinfo() << std::endl;

    mPipeUDP.bind(NULL, 2228);
    std::cout << __FUNCTION__ << ": pipe listening on UDP " << mPipeUDP.nameinfo() << std::endl;

    mPipeListen.listen(2228);
    std::cout << __FUNCTION__ << ": pipe listening on TCP " << mPipeListen.nameinfo() << std::endl;

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

    UnsubscribeAllFeeds(pSession);

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
        tsSocket tmpSocket(tsSocketType_TCP);

        while (!testCancel())
        {
            tsSocketSet socketSet;
            socketSet.addRdFD(mClientListen.fd());
            socketSet.addRdFD(mPipeListen.fd());
            socketSet.addRdFD(mPipeUDP.fd());

            for(std::vector<int>::const_iterator it = mPipeTCPConnections.begin(); it != mPipeTCPConnections.end(); it++)
                socketSet.addRdFD(*it);

            if (socketSet.select())
            {
                try
                {
                    if (socketSet.testRdFD(mPipeUDP.fd()))
                    {
                        tsTickReceiver::receive(mPipeUDP, 0);
                    }

                    for(std::vector<int>::iterator it = mPipeTCPConnections.begin(); it != mPipeTCPConnections.end(); )
                    {
                        if (socketSet.testRdFD(*it))
                        {
                            tmpSocket.attachFD(*it);

                            if (!tsTickReceiver::receive(tmpSocket, 0))
                            {
                                printf("%s: closing pipe TCP connection with fd %d\n", __FUNCTION__, *it);
                                tmpSocket.close();
                                it = mPipeTCPConnections.erase(it);
                                continue;
                            }

                            tmpSocket.detachFD();
                        }
                        it++;
                    }

                    if (socketSet.testRdFD(mPipeListen.fd()) && (mPipeTCPConnections.size() < MaxPipeConnections))
                    {
                        int newSocketPipe = mPipeListen.accept();
                        printf("%s: new pipe connection on %s with fd %d\n", __FUNCTION__, mPipeListen.nameinfo().c_str(), newSocketPipe);
                        mPipeTCPConnections.push_back(newSocketPipe);
                    }

                    if (socketSet.testRdFD(mClientListen.fd()) && (000 < MaxClientConnections)) // xxx count sessions
                    {
                        int newSocketOut = mClientListen.accept();
                        printf("%s: new client connection on %s with fd %d\n", __FUNCTION__, mClientListen.nameinfo().c_str(), newSocketOut);
                        CreateSession(newSocketOut);
                    }
                }
                catch(tsSocketException& e)
                {
                    tmpSocket.detachFD();
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

    // close all TCP connections for pipe
    for(std::vector<int>::reverse_iterator it = mPipeTCPConnections.rbegin(); it != mPipeTCPConnections.rend(); it++)
        ::close(*it);
    mPipeTCPConnections.clear();

    return NULL;
}

void tsNode::UnsubscribeAllFeeds(tsSession* pSession)
{
    // unsubscribe all feeds for this session
    for (SubscriberMap::iterator it=mSubscriberMap.begin(); it!=mSubscriberMap.end(); )
    {
        SubscriberMap::iterator it_inc = it; it_inc++;
        if (it->second == pSession)
            mSubscriberMap.erase(it);
        it = it_inc;
    }
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
        std::string node = mPipeUDP.nameinfo();
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

    tsMutexLocker lock(mNodeMutex);

    std::pair<SubscriberMap::const_iterator,SubscriberMap::const_iterator> range = mSubscriberMap.equal_range(feedID);
    for (SubscriberMap::const_iterator it=range.first; it!=range.second; ++it)
        it->second->SendOut(pRawTick, tickSize);
}

