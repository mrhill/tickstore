#include "tsStoreMemory.h"

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

