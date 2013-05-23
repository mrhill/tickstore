#ifndef tsSession_H
#define tsSession_H

#include "tsdef.h"
#include "tsStore.h"
#include "tsTickReceiver.h"

class tsSession : protected tsTickReceiver
{
    tsStore& mStore;

    virtual void Proc(const char* pRawTick, bbUINT tickSize);

public:
    tsSession(tsTickFactory& tickFactory, tsStore& store, int fd, int procID);
    ~tsSession();
};

#endif

