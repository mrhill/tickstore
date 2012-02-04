#ifndef tsDEF_H
#define tsDEF_H

#include <babel/defs.h>
#include <vector>

struct tsRawTick
{
    bbU8 mData[64];
};

enum tsType
{
    tsType_Char = 0,
    tsType_U8,
    tsType_U16,
    tsType_U24,
    tsType_U32,
    tsType_U64,
    tsType_F32,
    tsType_F64,
    tsTypeCount
};

#define tsTYPESIZES \
    1, /* tsType_Char */ \
    1, /* tsType_U8   */ \
    2, /* tsType_U16  */ \
    3, /* tsType_U24  */ \
    4, /* tsType_U32  */ \
    8, /* tsType_U64  */ \
    4, /* tsType_F32  */ \
    8, /* tsType_F64  */ \

#define tsTYPEALIGN \
    1, /* tsType_Char */ \
    1, /* tsType_U8   */ \
    2, /* tsType_U16  */ \
    1, /* tsType_U24  */ \
    4, /* tsType_U32  */ \
    8, /* tsType_U64  */ \
    4, /* tsType_F32  */ \
    8, /* tsType_F64  */ \


extern const bbU8 tsgTypeSize[tsTypeCount];
extern const bbU8 tsgTypeAlign[tsTypeCount];

template <class T> struct tsVecManagedPtr : public std::vector<T*>
{
    ~tsVecManagedPtr()
    {
        for(const_reverse_iterator it = rbegin(); it!=rend(); it++)
            delete *it;
    }
};

#endif

