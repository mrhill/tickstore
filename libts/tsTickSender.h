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
#include <stdio.h>

class tsTickSender
{
    bbU32       mTickCount;             //!< Tick count for this sender
    tsMutex     mConnectMutex;
    tsSocket    mSocket;
    int         mPort;
    std::string mHostName;
    std::string mQueueName;
    FILE*       mLogFD;
    bbU64       mUID;
    bbU8        mPwdHash[32];

    void connect();
    void tierdown();
    bool authenticate();

public:
    tsTickSender(const char* pQueueName, const char* pHostAdr, int port = 2227);
    ~tsTickSender();

    void setLogin(bbU64 uid, const char* pPwdHash);

    void send(tsTick& tick);
    inline tsTickSender& operator<<(tsTick& tick) { send(tick); return *this; }
};

#endif

