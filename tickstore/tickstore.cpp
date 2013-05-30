#include "tsStore.h"
#include "tsTickReceiver.h"
#include "tsTickSender.h"
#include <memory>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv)
{
    try
    {
        std::auto_ptr<tsStore> pTickerStore(tsStore::Create(tsStoreBackend_MySQL, "ticks"));

        tsTickSender sender("tickstore", "localhost");

        tsTickSubscribe subcribe;
        sender << subcribe;
    }
    catch(std::exception& e)
    {
        std::cout << e.what();
    }

    return 0;
}

