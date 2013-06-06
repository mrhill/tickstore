#include "tsSocket.h"
#include "tsStore.h"
#include "tsThread.h"
#include "tsTickClient.h"
#include "tsTickReceiver.h"
#include "tsTickSender.h"
#include <memory>
#include <iostream>
#include <stdexcept>

struct TickClient : tsTickClient
{
    TickClient(const char* pServerAddr = "localhost", int port = 2227) : tsTickClient("tickstore", pServerAddr, port) {}
    virtual void ProcessTick(const char* pRawTick, bbUINT tickSize);
};

void TickClient::ProcessTick(const char* pRawTick, bbUINT tickSize)
{
    printf("%s 0x%X\n", __FUNCTION__, tickSize);
}

int main(int argc, char** argv)
{
    std::auto_ptr<tsStore> pTickerStore(tsStore::Create(tsStoreBackend_MySQL, "ticks"));

    TickClient node;

    tsTickAuth auth;
    auth.setUID(0x1);
    node << auth;

    tsTickSubscribe subcribe;
    node << subcribe;

    //for(int i=0;;i++)
    //    printf("%d ",i), node << subcribe;

    tsTick t;
    node << t;

    node.join();
    return 0;
}

