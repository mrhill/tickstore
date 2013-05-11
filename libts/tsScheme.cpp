#include "tsScheme.h"
#include <babel/mem.h>

const bbU8 tsgTypeSize[tsTypeCount] = { tsTYPESIZES };
const bbU8 tsgTypeAlign[tsTypeCount] = { tsTYPEALIGN };

tsScheme::tsScheme()
{
    mSize = 0;
    bbMemClear(mFieldTypes, sizeof(mFieldTypes));
    bbMemClear(mFieldNames, sizeof(mFieldNames));
}

tsScheme::~tsScheme()
{
    for(int i=0; i<bbARRSIZE(mFieldNames); i++)
    {
        bbMemFree(mFieldNames[i]);
    }
}

bbUINT tsScheme::GetFieldSize(int fieldIndex)
{
    if ((bbUINT)fieldIndex < mSize)
        return tsgTypeSize[mFieldTypes[fieldIndex]];

    bbErrSet(bbEBADPARAM);
    return 0;
}

bbUINT tsScheme::GetFieldAlign(int fieldIndex)
{
    if ((bbUINT)fieldIndex < mSize)
        return tsgTypeAlign[mFieldTypes[fieldIndex]];

    bbErrSet(bbEBADPARAM);
    return 0;
}

int tsScheme::AddField(tsType type, const char* pName)
{
    int idx = mSize;

    if (idx >= tsSCHEME_MAXSIZE)
    {
        bbErrSet(bbEFULL);
        return -1;
    }

    #ifdef _WIN32
    char* pNameCopy = _strdup(pName);
    #else
    char* pNameCopy = strdup(pName);
    #endif
    if (!pNameCopy)
    {
        bbErrSet(bbENOMEM);
        return -1;
    }

    mFieldTypes[idx] = type;
    mFieldNames[idx] = pNameCopy;
    mSize++;
    return idx;
}

int tsScheme::FindField(const char* pName)
{
    char** ppNames = &mFieldNames[0];
    int schemeSize = mSize;

    for(int fieldIndex=0; fieldIndex<schemeSize; fieldIndex++)
    {
        if (ppNames[fieldIndex] && !strcmp(ppNames[fieldIndex], pName))
            return fieldIndex;
    }

    return -1;
}

