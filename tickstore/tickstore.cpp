#include "tsStore.h"
#include "tsTickClient.h"
#include <memory>
#include <iostream>
#include <stdexcept>

class TickClient : public tsTickClient
{
    std::auto_ptr<tsStore> mpStore;

    virtual void ProcessTick(const char* pRawTick, bbUINT tickSize);

public:
    TickClient(const char* pServerAddr = "localhost", int port = 2227);
};

TickClient::TickClient(const char* pServerAddr, int port)
  : tsTickClient("tickstore", pServerAddr, port)
{
    mpStore = std::auto_ptr<tsStore>(tsStore::Create(tsStoreBackend_MySQL, "ticks"));
}

void TickClient::ProcessTick(const char* pRawTick, bbUINT tickSize)
{
    tsTickUnion tickUnion;
    tsTick& tick = tickUnion;
    tsTickFactory::unserialize(pRawTick, &tick);
    std::cout << tick;

    mpStore->SaveTick(pRawTick, tickSize);
}

int main(int argc, char** argv)
{
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

