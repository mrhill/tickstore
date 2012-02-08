#ifndef tsSTOREFILE_H
#define tsSTOREFILE_H

#include "tsStore.h"
#include <map>
#include <vector>
#include <stdio.h>

class tsStoreFile : public tsStore
{
    typedef std::vector<char> Table;
    typedef std::map<tsObjID, Table*> ObjIDTableMap;
    ObjIDTableMap mObjID2Table;

    const char* mpFilePath;
    FILE* mhFile;

public:
    tsStoreFile(tsTickFactory& tickFactory, const char* pFilePath);

    virtual void SaveTick(const char* pRawTick, bbUINT tickSize);
};

#endif

