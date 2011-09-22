#include "tsHeader.h"

tsHeaderPage* tsHeaderPage::Alloc()
{
    tsHeaderPage* pPage = new tsHeaderPage;
    if (!pPage)
        bbErrSet(bbENOMEM);
    return pPage;
}

void tsHeaderPage::Free(tsHeaderPage* pPage)
{
    bbMemFree(pPage);
}

tsHeader::tsHeader(tsScheme* pScheme)
  : mpScheme(pScheme)
{
    bbMemClear(mIndexPage, sizeof(mIndexPage));
    bbMemClear(mIndexOffset, sizeof(mIndexOffset));
    bbMemClear(mPages, sizeof(mPages));

    SetScheme(pScheme);
}

tsHeader::~tsHeader()
{
    ClearScheme();
}

void tsHeader::ClearScheme()
{
    for(int i=0; i<bbARRSIZE(mPages); i++)
    {
        tsHeaderPage::Free(mPages[i]);
        mPages[i] = NULL;
    }

    bbMemClear(mIndexPage, sizeof(mIndexPage));
    bbMemClear(mIndexOffset, sizeof(mIndexOffset));

    mpScheme = NULL;
}

bbERR tsHeader::SetScheme(tsScheme* pScheme)
{
    ClearScheme();

    bbUINT page = 0;
    bbUINT offset = 0;
    bbUINT fieldIndex = 0;
    tsHeaderPage* pPage = NULL;

    while(fieldIndex < pScheme->GetSize())
    {
        bbUINT fieldAlign =  pScheme->GetFieldAlign(fieldIndex);
        if (!fieldAlign)
            return bbELAST;

        offset = (offset + (fieldAlign-1)) &~ (fieldAlign-1);

        if (offset >= tsHEADER_PAGESIZE)
        {
            if (++page >= tsHEADER_MAXPAGES)
                return bbErrSet(bbEFULL);
            offset = 0;
        }

        mIndexPage[fieldIndex] = page;
        mIndexOffset[fieldIndex] = offset;

        if (mPages[page] == NULL)
        {
            pPage = mPages[page] = tsHeaderPage::Alloc();
            if (!pPage)
                return bbELAST;
        }

        int fieldSize = pScheme->GetFieldSize(fieldIndex);
        if (!fieldSize)
            return bbELAST;

        offset += fieldSize;

        fieldIndex++;
    }

    return bbEOK;
}

void tsHeader::Clear()
{
    for(int i=0; i<bbARRSIZE(mPages); i++)
    {
        if (mPages[i])
            mPages[i]->Clear();
    }
}

char tsHeader::GetFieldChar(int fieldIndex)
{
    tsHeaderPage* pPage = mPages[mIndexPage[fieldIndex]];
    bbU8* pData = &pPage->mData[mIndexOffset[fieldIndex]];
    return (char)*pData;
}

bbU8 tsHeader::GetFieldU8(int fieldIndex)
{
    tsHeaderPage* pPage = mPages[mIndexPage[fieldIndex]];
    bbU8* pData = &pPage->mData[mIndexOffset[fieldIndex]];
    return *pData;
}

bbU16 tsHeader::GetFieldU16(int fieldIndex)
{
    tsHeaderPage* pPage = mPages[mIndexPage[fieldIndex]];
    bbU8* pData = &pPage->mData[mIndexOffset[fieldIndex]];
    return *(bbU16*)pData;
}

bbU32 tsHeader::GetFieldU24(int fieldIndex)
{
    tsHeaderPage* pPage = mPages[mIndexPage[fieldIndex]];
    bbU8* pData = &pPage->mData[mIndexOffset[fieldIndex]];
    return bbLD24LE(pData);
}

bbU32 tsHeader::GetFieldU32(int fieldIndex)
{
    tsHeaderPage* pPage = mPages[mIndexPage[fieldIndex]];
    bbU8* pData = &pPage->mData[mIndexOffset[fieldIndex]];
    return *(bbU32*)pData;
}

bbU64 tsHeader::GetFieldU64(int fieldIndex)
{
    tsHeaderPage* pPage = mPages[mIndexPage[fieldIndex]];
    bbU8* pData = &pPage->mData[mIndexOffset[fieldIndex]];
    return *(bbU64*)pData;
}

float tsHeader::GetFieldF32(int fieldIndex)
{
    tsHeaderPage* pPage = mPages[mIndexPage[fieldIndex]];
    bbU8* pData = &pPage->mData[mIndexOffset[fieldIndex]];
    return *(float*)pData;
}

double tsHeader::GetFieldF64(int fieldIndex)
{
    tsHeaderPage* pPage = mPages[mIndexPage[fieldIndex]];
    bbU8* pData = &pPage->mData[mIndexOffset[fieldIndex]];
    return *(double*)pData;
}

void tsHeader::SetField(int fieldIndex, char val)
{
    tsHeaderPage* pPage = mPages[mIndexPage[fieldIndex]];
    bbU8* pData = &pPage->mData[mIndexOffset[fieldIndex]];
    *pData = val;
}

void tsHeader::SetField(int fieldIndex, bbU8 val)
{
    tsHeaderPage* pPage = mPages[mIndexPage[fieldIndex]];
    bbU8* pData = &pPage->mData[mIndexOffset[fieldIndex]];
    *pData = val;
}

void tsHeader::SetField(int fieldIndex, bbU16 val)
{
    tsHeaderPage* pPage = mPages[mIndexPage[fieldIndex]];
    bbU8* pData = &pPage->mData[mIndexOffset[fieldIndex]];
    *(bbU16*)pData = val;
}

void tsHeader::SetFieldU24(int fieldIndex, bbU32 val)
{
    tsHeaderPage* pPage = mPages[mIndexPage[fieldIndex]];
    bbU8* pData = &pPage->mData[mIndexOffset[fieldIndex]];
    bbST24LE(pData, val);
}

void tsHeader::SetField(int fieldIndex, bbU32 val)
{
    tsHeaderPage* pPage = mPages[mIndexPage[fieldIndex]];
    bbU8* pData = &pPage->mData[mIndexOffset[fieldIndex]];
    *(bbU32*)pData = val;
}

void tsHeader::SetField(int fieldIndex, bbU64 val)
{
    tsHeaderPage* pPage = mPages[mIndexPage[fieldIndex]];
    bbU8* pData = &pPage->mData[mIndexOffset[fieldIndex]];
    *(bbU64*)pData = val;
}

void tsHeader::SetField(int fieldIndex, float val)
{
    tsHeaderPage* pPage = mPages[mIndexPage[fieldIndex]];
    bbU8* pData = &pPage->mData[mIndexOffset[fieldIndex]];
    *(float*)pData = val;
}

void tsHeader::SetField(int fieldIndex, double val)
{
    tsHeaderPage* pPage = mPages[mIndexPage[fieldIndex]];
    bbU8* pData = &pPage->mData[mIndexOffset[fieldIndex]];
    *(double*)pData = val;
}

