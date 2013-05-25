#include "tsStoreMemory.h"
#include "tsTickReceiver.h"
#include "tsThread.h"
#include "tsTickSender.h"
#include "tsTickFinance.h"
#include "tsTracker.h"
#include "tsNode.h"
#include <memory>
#include <vector>
#include <iostream>
#include <stdexcept>

struct TestSendThread : public tsThread
{
    bbU64 mTestUID;

    TestSendThread(tsStore* pStore)
    {
        bbU8 pwdHash[32];
        memset(pwdHash, 0, sizeof(pwdHash));
        mTestUID = pStore->CreateUser("testsender", pwdHash);
    }

    virtual void* run()
    {
        msleep(2000);
        tsTickFactoryFinance factory;

        bbU64 sym = 0xDEADDEADUL;
        sym = (sym<<32) | 0xF00DF00DUL;
        tsObjID objID(0xE400002E, sym);

        std::cout << tsTime::current().str() << std::endl;

        tsTickPriceVolume priceTick(objID, 23.42, (bbU64)68<<32, 0xDF);

        try
        {
            tsTickSender sender(factory, "tickschleuder", "localhost");

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
    tsTickFactoryFinance factory;
    tsTracker tracker;

    try
    {
        std::auto_ptr<tsStore> pTickerStore(tsStore::Create(factory, tsStoreBackend_MySQL, "ticks"));
        tsNode node(factory, tracker, *pTickerStore);

        //TestSendThread sender(pTickerStore.get());
        //sender.start();

        node.join();
        //sender.join();
    }
    catch(std::exception& e)
    {
        std::cout << e.what();
    }

    return 0;
}

