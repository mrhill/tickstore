#ifndef tsTICKPROCSCHLEUDER_H
#define tsTICKPROCSCHLEUDER_H

#include "tsTickReceiver.h"
#include "tsMon.h"
#include "tsObjID.h"
#include <map>

class tsTickProcSchleuder : public tsTickReceiver
{
    typedef std::map<tsObjID, tsMonSymbol*> ObjIDMonMap;
    ObjIDMonMap mObjID2Mon;
public:
    tsTickProcSchleuder(tsTickFactory& tickFactory, tsStore& store, int socketFD, int procID)
      : tsTickReceiver(tickFactory, store, socketFD, procID)
    {
    }

    tsMonSymbol* GetMon(const tsObjID& objID);

    virtual void Proc_(const tsTick& tick);
};

#endif

