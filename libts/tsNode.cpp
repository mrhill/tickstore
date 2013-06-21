#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <babel/algo.h>
#include "tsNode.h"

tsNode::tsNode(zmq::context_t& zmq, tsTracker& tracker)
  : mZmq(zmq),
    mAuthSocket(zmq, ZMQ_REQ),
    mTracker(tracker),
    mClientListen(tsSocketType_TCP),
    mPipeListen(tsSocketType_TCP),
    mNextSessionID(0)
{
    size_t size = sizeof(mAuthSocketFD);
    mAuthSocket.connect("inproc://auth");
    mAuthSocket.getsockopt(ZMQ_FD, &mAuthSocketFD, &size);

    mClientListen.listen(2227);
    std::cout << __FUNCTION__ << ": listening on TCP " << mClientListen.nameinfo() << std::endl;

    mPipeListen.listen(2228);
    std::cout << __FUNCTION__ << ": pipe listening on TCP " << mPipeListen.nameinfo() << std::endl;
}

tsNode::~tsNode()
{
}

void tsNode::DestroySession(tsSession* pSession)
{
    printf("%s %d: closing client TCP connection with fd %d\n", __FUNCTION__, pSession->sessionID(), pSession->socketFD());
    UnsubscribeAllFeeds(pSession);
    delete pSession;
}

void* tsNode::run()
{
    while (true)
    {
        for(;;)
        {
            int zevents = 0;
            size_t zevents_len = sizeof(zevents);
            mAuthSocket.getsockopt(ZMQ_EVENTS, &zevents, &zevents_len);
            if (!(zevents & ZMQ_POLLIN))
                break;

            char msg[tsTick::SERIALIZEDMAXSIZE];
            int msgSize = mAuthSocket.recv(msg, sizeof(msg), ZMQ_DONTWAIT);
            if (msgSize < tsTick::SERIALIZEDHEADSIZE)
            {
                printf("tsNode::run: invalid control message size %d\n", msgSize);
                break;
            }
            ProcessControlMsg(msg, msgSize);
        }

        tsSocketSet socketSet;
        socketSet.addRdFD(mAuthSocketFD);
        socketSet.addRdFD(mClientListen.fd());
        socketSet.addRdFD(mPipeListen.fd());

        for(std::list<tsTickReceiver*>::const_iterator it = mPipeTCPConnections.begin(); it != mPipeTCPConnections.end(); ++it)
            socketSet.addRdFD((*it)->socketFD());

        for(std::vector<tsSession*>::iterator it = mSessions.begin(); it != mSessions.end(); )
        {
            if ((*it)->socketFD() == -1)
            {
                DestroySession(*it);
                it = mSessions.erase(it);
            }
            else
            {
                socketSet.addRdFD((*it)->socketFD());
                ++it;
            }
        }

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
                            DestroySession(*it);
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
            catch(std::runtime_error& e)
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

void tsNode::Authenticate(int sessionID, const char* pRawAuthTick, bbUINT tickSize)
{
    if (tsTick::unserializeHead_peekTickType(pRawAuthTick) != tsTickType_Auth)
        throw std::runtime_error("tsNode::Authenticate expected tsTickType_Auth");

    zmq::message_t msg(tickSize);

    char* d = (char*)msg.data();
    memcpy(d, pRawAuthTick, tickSize);
    tsTick::serializeHead_pokeQueryID(d, sessionID);

    mAuthSocket.send(msg);
}

void tsNode::ProcessControlMsg(const char* pMsg, bbUINT msgSize)
{
    tsTickUnion tick;
    tsTickType tt = tsTick::unserializeHead_peekTickType(pMsg);
    switch(tt)
    {
    case tsTickType_AuthReply:
        {
            tsTickFactory::unserialize(pMsg, &tick);
            tsTickAuthReply& authReply = static_cast<tsTickAuthReply&>((tsTick&)tick);

            int sessionID = (int)authReply.queryID();

            tsSession** pSession = (tsSession**)bbBSearch(&sessionID, &mSessions.front(), mSessions.size(), sizeof(tsSession*), tsSession::cmpSessionID);
            printf("%s: session ID %d (pSession %p), auth success: %d\n", __FUNCTION__, sessionID, pSession, authReply.success());
            if (pSession)
            {
                (*pSession)->SendTick(authReply);
                if (!authReply.success())
                    (*pSession)->close();
            }
        }
        break;
    default:
        printf("%s: invalid tick type %d\n", __FUNCTION__, tt);
        break;
    }
}

