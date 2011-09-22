#ifndef tsSCHEME_H
#define tsSCHEME_H

#include "tsdef.h"

#define tsSCHEME_MAXSIZE 64

class tsScheme
{
    bbUINT mSize;
    bbU8   mFieldTypes[tsSCHEME_MAXSIZE];
    char*  mFieldNames[tsSCHEME_MAXSIZE];

public:
    tsScheme();
    ~tsScheme();

    bbUINT GetSize() const { return mSize; }
    bbUINT GetFieldSize(int fieldIndex);
    bbUINT GetFieldAlign(int fieldIndex);
    int AddField(tsType type, const char* pName);
    int FindField(const char* pName);
};

#endif

