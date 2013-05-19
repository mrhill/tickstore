#ifndef tsTICKSENDER_H
#define tsTICKSENDER_H

#include "tsTick.h"
#include "tsTickQueue.h"
#include "tsMutex.h"
#include "tsSemaphore.h"
#include "tsThread.h"
#include "tsSocket.h"
#include <queue>
#include <string>

class tsTickSender : protected tsThread
{
    bbU32       mTickCount;             //!< Tick count for this sender
    tsTickQueue mTickQueue;             //!< Tick send queue
    tsSemaphore mTickQueueSema;
    tsMutex     mTickQueueWriteMutex;   //!< Mutex to protect write access to mTickQueue from multiple threads
    tsSocket    mSocket;
    int         mLogLevel;
    int         mPort;
    std::string mHostName;

    virtual void* run();
    void tierDownConnection();
    void sendUnprotected(tsTick& tick);
    void sendDiagTick();

public:
    tsTickSender(tsTickFactory& tickFactory, const char* pQueueName, const char* pHostAdr, int port = 2227);
    ~tsTickSender();

    void send(tsTick& tick);
    inline tsTickSender& operator<<(tsTick& tick) { send(tick); return *this; }
};

#endif

