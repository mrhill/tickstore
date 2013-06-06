#ifndef tsTICKCLIENT_H
#define tsTICKCLIENT_H

#include "tsTick.h"
#include "tsThread.h"
#include "tsTickReceiver.h"

class tsTickClient : public tsThread, public tsTickListener, public tsTickReceiver
{
    int         mServerPort;
    std::string mServerAddr;
    bbU32       mTickCount;

    virtual void* run();
public:
    tsTickClient(const char* pQueueName, const char* pServerAddr, int port = 2227);
    ~tsTickClient();

    void send(tsTick& tick);
    inline tsTickClient& operator<<(tsTick& tick) { send(tick); return *this; }

    virtual void ProcessTick(const char* pRawTick, bbUINT tickSize);
};

#endif

