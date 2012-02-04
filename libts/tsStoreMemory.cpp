#include "tsStoreMemory.h"
#include <iostream>

void tsStoreMemory::SaveTick(const char* pRawTick)
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
        int tickSize = mTickFactory.serializedSize(tick);
        pTable->reserve(tickSize);
        do { pTable->push_back(*pRawTick++); } while (--tickSize);
    }
}

bbERR tsStoreMemory::GetHeader(const tsObjID& objID, tsHeader& header)
{
    return bbEOK;
}

bbERR tsStoreMemory::SetHeader(const tsHeader& header)
{
    ObjIDHeaderMap::iterator it = mObjID2Header.find(header.GetObjID());

    if (it == mObjID2Header.end())
    {
        mObjID2Header[header.GetObjID()] = new tsHeader(header);
    }
    else
    {
        *it->second = header;
    }
        
    return bbEOK;
}

bbERR tsStoreMemory::SaveHeader(tsHeader& header)
{
    return 0;
}

