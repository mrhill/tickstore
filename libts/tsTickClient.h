#ifndef tsTICKCLIENT_H
#define tsTICKCLIENT_H

#include "tsTick.h"
#include "tsTickQueue.h"
#include "tsThread.h"
#include "tsSocket.h"
#include "tsSemaphore.h"

class tsTickClient : protected tsThread
{
    tsSocket    mSocket;
    int         mServerPort;
    std::string mServerAddr;
    tsTickQueue mTickQueue;
    tsSemaphore mTickQueueSema;

    virtual void* run();
public:
    tsTickClient(const char* pQueueName, const char* pServerAddr, int port = 2227);
    ~tsTickClient();
};

#endif

