#include "tsStoreMemory.h"
#include "tsTickReceiver.h"
#include "tsThread.h"
#include "tsTickSender.h"
#include "tsTickFinance.h"
#include "tsTickProcSchleuder.h"
#include <memory>
#include <vector>
#include <iostream>
#include <stdexcept>

class TestSendThread : public tsThread
{
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
            auth.setUID(1);
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
    TestSendThread sender;
    sender.start();

    int procID = 0;
    try
    {
        std::auto_ptr<tsStore> pTickerStore(tsStore::Create(factory, tsStoreBackend_MySQL, "ticks"));
        tsVecManagedPtr<tsTickProcSchleuder> tickProcessors;

        int port = 2227;
        std::cout << __FUNCTION__ << ": listening for connections on port " << port << std::endl;
        tsSocket listenSocket(tsSocketType_TCP);
        listenSocket.listen(port);
        while (true)
        {
            int newSocket = listenSocket.accept();
            tickProcessors.push_back(new tsTickProcSchleuder(factory, *pTickerStore, newSocket, ++procID, 0));
        }
    }
    catch(std::exception& e)
    {
        std::cout << e.what();
    }

    sender.join();
    return 0;
}

