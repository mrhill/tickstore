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
    bbU32       mTickCount;
    tsTickQueue mTickQueue;
    tsSemaphore mTickQueueSema;
    tsMutex     mTickQueueWriteMutex;
    tsSocket    mSocket;
    int         mLogLevel;
    int         mPort;
    std::string mHostName;

    void* run();
    void tierDownConnection();
    void sendUnprotected(tsTick& tick);
    void sendDiagTick();

public:
    tsTickSender(const char* pHostAdr, int port = 2227);
    ~tsTickSender();

    void send(tsTick& tick);
    inline tsTickSender& operator<<(tsTick& tick) { send(tick); return *this; }
};

#endif

