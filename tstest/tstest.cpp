#include "tsTickSender.h"
#include <memory>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv)
{
    bbU64 sym = 0xDEADDEADUL;
    sym = (sym<<32) | 0xF00DF00DUL;
    tsObjID objID(0xE400002E, sym);
    tsTick tick;

    std::cout << tsTime::current().str() << std::endl;

    tsTickPrice priceTick(objID, 23.42, 0);

    try
    {
        tsTickSender sender("tstest",
                            argc>1 ? argv[1] : "localhost",
                            argc>2 ? atoi(argv[2]) : 2227);

        tsTickAuth auth;
        auth.setUID(1);
        memset(auth.mPwdHash, 0, sizeof(auth.mPwdHash));
        sender << auth;

        tsTickSubscribe subscr;
        subscr.setFeedID(0);
        sender << subscr;

        for (int i=0; i<1000; i++)
        {
            priceTick.setPrice(i);
            sender << tick << priceTick;
        }
    }
    catch(std::exception& e)
    {
        std::cout << e.what();
    }

    return 0;
}

