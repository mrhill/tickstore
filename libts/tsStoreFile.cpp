#include "tsStoreFile.h"
#include <iostream>
#include <stdexcept>
#include <errno.h>

tsStoreFile::tsStoreFile(tsTickFactory& tickFactory, const char* pFilePath)
  : tsStore(tickFactory), mpFilePath(pFilePath), mhFile(NULL)
{
    mhFile = fopen(pFilePath, "ab");
    if (!mhFile)
        throw std::runtime_error(strprintf("%s: Error %d opening file '%s'\n", __FUNCTION__, errno, pFilePath));
}

tsStoreFile::~tsStoreFile()
{
    if (mhFile)
        fclose(mhFile);
}

void tsStoreFile::SaveTick(const char* pRawTick, bbUINT tickSize)
{
    if (fwrite(pRawTick, 1, tickSize, mhFile) != tickSize)
        printf("%s: error writing %d bytes to '%s'\n", __FUNCTION__, tickSize, mpFilePath);

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

