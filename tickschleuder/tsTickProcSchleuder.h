#ifndef tsTICKPROCSCHLEUDER_H
#define tsTICKPROCSCHLEUDER_H

#include "tsTickProc.h"
#include "tsMon.h"
#include "tsObjID.h"
#include <map>

class tsTickProcSchleuder : public tsTickProc
{
    typedef std::map<tsObjID, tsMonSymbol*> ObjIDMonMap;
    ObjIDMonMap mObjID2Mon;
public:
    tsTickProcSchleuder(tsTickFactory& tickFactory, tsStore& store, int socketFD, int procID)
      : tsTickProc(tickFactory, store, socketFD, procID)
    {
    }

    tsMonSymbol* GetMon(const tsObjID& objID);

    virtual void Proc(const tsTick& tick);
};

#endif
