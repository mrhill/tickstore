#include "tsSession.h"

tsSession::tsSession(tsTickFactory& tickFactory, tsStore& store, int fd, int procID)
  : tsTickReceiver(tickFactory, fd, procID), mStore(store)
{
    tsTickReceiver::start();
}

tsSession::~tsSession()
{
}

void tsSession::Proc(const char* pRawTick, bbUINT tickSize)
{
    tsTickUnion tickUnion;
    tsTick& tick = tickUnion;
    mStore.tickFactory().unserialize(pRawTick, &tick);

    switch (tick.type())
    {
    case tsTickType_Auth:
        printf("%s: tsTickType_Auth\n", __FUNCTION__);
        break;
    }
}


