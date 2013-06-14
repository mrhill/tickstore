#include "tsNode.h"
#include <stdexcept>
#include <iostream>
#include <algorithm>

tsNode::tsNode(tsTracker& tracker)
  : mTracker(tracker),
    mClientListen(tsSocketType_TCP),
    mPipeListen(tsSocketType_TCP),
    mNextSessionID(0)
{
    mClientListen.listen(2227);
    std::cout << __FUNCTION__ << ": listening on TCP " << mClientListen.nameinfo() << std::endl;

    mPipeListen.listen(2228);
    std::cout << __FUNCTION__ << ": pipe listening on TCP " << mPipeListen.nameinfo() << std::endl;
}

tsNode::~tsNode()
{
}

void* tsNode::run()
{
    while (true)
    {
        tsSocketSet socketSet;
        socketSet.addRdFD(mClientListen.fd());
        socketSet.addRdFD(mPipeListen.fd());

        for(std::list<tsTickReceiver*>::const_iterator it = mPipeTCPConnections.begin(); it != mPipeTCPConnections.end(); ++it)
            socketSet.addRdFD((*it)->socketFD());

        for(std::vector<tsSession*>::const_iterator it = mSessions.begin(); it != mSessions.end(); ++it)
            socketSet.addRdFD((*it)->socketFD());

        if (socketSet.select())
        {
            try
            {
                for(std::list<tsTickReceiver*>::iterator it = mPipeTCPConnections.begin(); it != mPipeTCPConnections.end(); )
                {
                    if (socketSet.testRdFD((*it)->socketFD()))
                    {
                        if (!(*it)->receiveTicks(0))
                        {
                            printf("%s: closing pipe TCP connection with fd %d\n", __FUNCTION__, (*it)->socketFD());
                            delete *it;
                            it = mPipeTCPConnections.erase(it);
                            continue;
                        }
                    }
                    ++it;
                }

                for(std::vector<tsSession*>::iterator it = mSessions.begin(); it != mSessions.end(); )
                {
                    if (socketSet.testRdFD((*it)->socketFD()))
                    {
                        if (!(*it)->receiveTicks(0))
                        {
                            printf("%s: closing client TCP connection with fd %d\n", __FUNCTION__, (*it)->socketFD());
                            UnsubscribeAllFeeds(*it);
                            delete *it;
                            it = mSessions.erase(it);
                            continue;
                        }
                    }
                    ++it;
                }

                if (socketSet.testRdFD(mPipeListen.fd()) && (mPipeTCPConnections.size() < MaxPipeConnections))
                {
                    int newSocketPipe = mPipeListen.accept();
                    printf("%s: new pipe connection on %s with fd %d\n", __FUNCTION__, mPipeListen.nameinfo().c_str(), newSocketPipe);
                    tsTickReceiver* pReceiver = new tsTickReceiver(this, newSocketPipe);
                    mPipeTCPConnections.push_back(pReceiver);
                }

                if (socketSet.testRdFD(mClientListen.fd()) && (mSessions.size() < MaxClientConnections))
                {
                    int newSocketOut = mClientListen.accept();
                    printf("%s: new client connection on %s with fd %d\n", __FUNCTION__, mClientListen.nameinfo().c_str(), newSocketOut);
                    tsSession* pSession = new tsSession(*this, newSocketOut, mNextSessionID++);
                    mSessions.push_back(pSession);
                }
            }
            catch(tsSocketException& e)
            {
                printf("%s: %s\n", __FUNCTION__, e.what());
            }
        }
    }

    printf("%s: shutting down\n", __FUNCTION__);

    // close all TCP connections
    for(std::list<tsTickReceiver*>::reverse_iterator it = mPipeTCPConnections.rbegin(); it != mPipeTCPConnections.rend(); ++it)
        delete (*it);
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

void tsNode::SubscribeFeed(bbU64 feedID, bbU64 queryID, tsSession* pSession)
{
    SubscriberKey key = SubscriberKey(feedID, queryID);
    std::pair<SubscriberMap::iterator,SubscriberMap::iterator> range = mSubscriberMap.equal_range(key);

    if (range.first != range.second) // node already subscribing to feedID?
    {
        // session already subscribing to feedID?
        for (SubscriberMap::const_iterator it=range.first; it!=range.second; ++it)
            if (it->second == pSession)
                return;
    }
    else if (queryID == 0) // rt feed or query?
    {
        // subscribe node to feedID
        std::string node = mPipeListen.nameinfo();
        std::cout << __FUNCTION__ << ": subscribing node for feedID " << feedID << std::endl;
        if (node.size())
            mTracker.Subscribe(node, feedID);
    }

    // map feed to session
#if __cplusplus > 199711L
    mSubscriberMap.insert(range.second, std::pair<SubscriberKey, tsSession*>(key, pSession));
#else
    mSubscriberMap.insert(range.first, std::pair<SubscriberKey, tsSession*>(key, pSession));
#endif
}

void tsNode::ProcessTick(const char* pRawTick, bbUINT tickSize)
{
    SubscriberKey key = SubscriberKey(tsTick::unserializeHead_peekFeedID(pRawTick), tsTick::unserializeHead_peekQueryID(pRawTick));

    std::pair<SubscriberMap::const_iterator,SubscriberMap::const_iterator> range = mSubscriberMap.equal_range(key);
    for (SubscriberMap::const_iterator it=range.first; it!=range.second; ++it)
        it->second->SendTick(pRawTick, tickSize);
}
