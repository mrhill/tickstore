#include "tsdef.h"
#include <babel/StrBuf.h>

static std::string strprintf(const char* fmt, bbVALIST args)
{
    bbStrBuf str;
    str.Printf(fmt, args);
    return std::string(str.GetPtr());
}

std::string strprintf(const char* fmt, ...)
{
    bbVALIST args;
    bbVASTART(args, fmt);
    const std::string ret = strprintf(fmt, args);
    bbVAEND(args);
    return ret;
}

