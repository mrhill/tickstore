#ifndef tsTICKPROCSCHLEUDER_H
#define tsTICKPROCSCHLEUDER_H

#include "tsTickReceiver.h"
#include "tsStore.h"
#include "tsMon.h"
#include "tsObjID.h"
#include <map>

class tsTickProcSchleuder : public tsTickReceiver
{
    tsStore&    mStore;
    bbUINT      mGroup;



    typedef std::map<tsObjID, tsMonSymbol*> ObjIDMonMap;
    ObjIDMonMap mObjID2Mon;
public:
    tsTickProcSchleuder(tsTickFactory& tickFactory, tsStore& store, int socketFD, int procID, bbUINT group);

    tsMonSymbol* GetMon(const tsObjID& objID);

    virtual void Proc(const char* pRawTick, bbUINT tickSize);
    virtual void Proc_(const tsTick& tick);
};

#endif

