#include "tsStoreMemory.h"
#include <iostream>

void tsStoreMemory::SaveTick(const char* pRawTick, bbUINT /*tickSize*/)
{
    tsTickUnion tickUnion;
    tsTick& tick = static_cast<tsTick&>(tickUnion);
    tick.unserializeHead(pRawTick);

    if (tick.type() != tsTickType_Diag)
    {
        Table* pTable;

        ObjIDTableMap::iterator it = mObjID2Table.find(tick.objID());
        if (it == mObjID2Table.end())
        {
            mObjID2Table[tick.objID()] = pTable = new Table;
            std::cout << __FUNCTION__ << ": Created new table for object ID " << tick.objID().str() << std::endl;
        }
        else
        {
            pTable = it->second;
        }

        //xxx
        int tickSize = tsTickFactory::serializedSize(tick);
        pTable->reserve(tickSize);
        do { pTable->push_back(*pRawTick++); } while (--tickSize);
    }
}

