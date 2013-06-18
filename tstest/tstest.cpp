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
        memset(auth.mPwdHash, 1, sizeof(auth.mPwdHash));
        bool authResult = sender.authenticate(auth.UID(), (const char*)auth.mPwdHash);
        printf("authenticate result for UID 0x%"bbI64"X: %d\n", auth.UID(), authResult);

        tsTickSubscribe subscr;
        subscr.setFeedID(0);
        sender << subscr;

        for(int loop=0;; loop++)
        {
            printf("%s: loop %d\n", __FUNCTION__, loop);
            for (int i=0; i<1000; i++)
            {
                priceTick.setPrice(i);
                sender << tick << priceTick;
            }
            tsThread::msleep(2000);
        }
    }
    catch(std::exception& e)
    {
        std::cout << e.what();
    }

    printf("%s: exit\n", __FUNCTION__);

    return 0;
}

