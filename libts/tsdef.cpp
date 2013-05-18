#include "tsdef.h"
#include <babel/StrBuf.h>

std::string strprintf(const char* fmt, ...)
{
    bbStrBuf str;

    bbVALIST args;
    bbVASTART(args, fmt);
    str.VPrintf(fmt, args);
    bbVAEND(args);
    return std::string(str.GetPtr());
}

