#ifndef tsHEADER_H
#define tsHEADER_H

#include "tsObjID.h"
#include "tsScheme.h"
#include <babel/mem.h>

#define tsHEADER_MAXPAGES 4
#define tsHEADER_PAGESIZE 256

struct tsHeaderPage
{
    bbU8 mData[tsHEADER_PAGESIZE];
    inline void Clear() { bbMemClear(mData, sizeof(mData)); }

    static tsHeaderPage* Alloc();
    static void Free(tsHeaderPage*);
};

class tsHeader
{
    tsObjID       mObjID;                           //12
    tsScheme*     mpScheme;                         //4
    bbU8          mIndexPage[tsSCHEME_MAXSIZE];     //64
    bbU8          mIndexOffset[tsSCHEME_MAXSIZE];   //64
    tsHeaderPage* mPages[tsHEADER_MAXPAGES];        //16 -> 160 bytes

    void ClearScheme();
    bbERR SetScheme(tsScheme* pScheme);

public:
    tsHeader(tsScheme* pScheme);
    ~tsHeader();

    void SetObjID(const tsObjID& objID) { mObjID = objID; }
    const tsObjID& GetObjID() const { return mObjID; }

    void Clear();

    int FindField(const char* pName) { return mpScheme->FindField(pName); }

    char   GetFieldChar(int fieldIndex);
    bbU8   GetFieldU8  (int fieldIndex);
    bbU16  GetFieldU16 (int fieldIndex);
    bbU32  GetFieldU24 (int fieldIndex);
    bbU32  GetFieldU32 (int fieldIndex);
    bbU64  GetFieldU64 (int fieldIndex);
    float  GetFieldF32 (int fieldIndex);
    double GetFieldF64 (int fieldIndex);

    void SetField(int fieldIndex, char val);
    void SetField(int fieldIndex, bbU8 val);
    void SetField(int fieldIndex, bbU16 val);
    void SetFieldU24(int fieldIndex, bbU32 val);
    void SetField(int fieldIndex, bbU32 val);
    void SetField(int fieldIndex, bbU64 val);
    void SetField(int fieldIndex, float val);
    void SetField(int fieldIndex, double val);
};

#endif

