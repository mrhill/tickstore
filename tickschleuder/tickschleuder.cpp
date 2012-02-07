#include "tsStoreMemory.h"
#include "tsTickProcSchleuder.h"
#include "tsThread.h"
#include "tsTickSender.h"
#include "tsTickFinance.h"
#include <memory>
#include <vector>
#include <iostream>
#include <stdexcept>

class TestSendThread : public tsThread
{
    virtual void* run()
    {
        tsTickFactoryFinance factory;

        bbU64 sym = 0xDEADDEADUL;
        sym = (sym<<32) | 0xF00DF00DUL;
        tsObjID objID(0xE400002E, sym);
        tsTick tick;

        std::cout << tsTime::current().str() << std::endl;

        tsTickPriceVolume priceTick(objID, 23.42, (bbU64)68<<32, 0xDF);

        try
        {
            tsTickSender sender(factory, "localhost");

            for (int i=0; i<1000; i++)
            {
                sender << tick << priceTick;
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
        std::auto_ptr<tsStore> pTickerStore(tsStore::Create(factory, tsStoreBackend_Memory));
        tsVecManagedPtr<tsTickProc> tickProcessors;

        int port = 2227;
        std::cout << __FUNCTION__ ": listening for connections on port " << port << std::endl;
        tsSocket listenSocket(tsSocketType_TCP);
        listenSocket.listen(port);
        while (true)
        {
            int newSocket = listenSocket.accept();
            tickProcessors.push_back(new tsTickProcSchleuder(factory, *pTickerStore, newSocket, ++procID));
        }
    }
    catch(std::exception& e)
    {
        std::cout << e.what();
    }

    sender.join();
    return 0;
}
   
