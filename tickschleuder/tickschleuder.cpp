#include <memory>
#include <iostream>
#include <stdexcept>
#include "tsdef.h"
#include "tsThread.h"
#include "tsTickSender.h"
#include "tsTracker.h"
#include "tsNode.h"
#include "tsAuth.h"
#include "tsStore.h"

struct TestSendThread : public tsThread
{
    bbU64 mTestUID;

    TestSendThread(tsStore* pStore)
    {
        bbU8 pwdHash[32];
        memset(pwdHash, 0, sizeof(pwdHash));
        mTestUID = tsAuth::instance().CreateUser("testsender", pwdHash);
    }

    virtual void* run()
    {
        msleep(2000);

        bbU64 sym = 0xDEADDEADUL;
        sym = (sym<<32) | 0xF00DF00DUL;
        tsObjID objID(0xE400002E, sym);

        std::cout << tsTime::current().str() << std::endl;

        tsTickPriceVolume priceTick(objID, 23.42, (bbU64)68<<32, 0xDF);

        try
        {
            tsTickSender sender("tickschleuder", "localhost");

            tsTickAuth auth;
            auth.setUID(mTestUID);
            memset(auth.mPwdHash, 0, sizeof(auth.mPwdHash));
            sender << auth;

            for (int i=0; i<100; i++)
            {
                sender << priceTick;
                priceTick.setPrice(i);
                priceTick.setVolume(1<<(i&63));
            }
        }
        catch(std::exception& e)
        {
            std::cout << e.what();
        }

        return 0;
    }
};

int main(int argc, char** argv)
{
    tsTracker tracker;

    tsAuthMySQL auth("ticks");

    try
    {
        std::auto_ptr<tsStore> pTickerStore(tsStore::Create(tsStoreBackend_MySQL, "ticks"));

        tsNode node(tracker);

        TestSendThread sender(pTickerStore.get());
        if (argc > 1 && !strcmp(argv[1], "-t"))
            sender.start();

        node.run();
        sender.join();
    }
    catch(std::exception& e)
    {
        std::cout << e.what();
    }

    return 0;
}

