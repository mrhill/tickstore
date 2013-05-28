#include "tsStore.h"
#include "tsTickReceiver.h"
#include "tsTickSender.h"
#include "tsTickFinance.h"
#include <memory>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv)
{
    try
    {
        tsTickFactoryFinance factory;
        std::auto_ptr<tsStore> pTickerStore(tsStore::Create(factory, tsStoreBackend_MySQL, "ticks"));

        tsTickSender sender(factory, "tickstore", "localhost");

        tsTickSubscribe subcribe;
        sender << subcribe;
    }
    catch(std::exception& e)
    {
        std::cout << e.what();
    }

    return 0;
}

