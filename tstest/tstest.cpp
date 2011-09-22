#include "tsStoreMemory.h"
#include "tsTickSender.h"
#include "tsTick.h"
#include <memory>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv)
{
    std::auto_ptr<tsStore> pTickerStore(tsStore::Create(tsStoreBackend_Memory));

    bbU64 sym = 0xDEADDEADUL;
    sym = (sym<<32) | 0xF00DF00DUL;
    tsObjID objID(0xE400002E, sym);
    tsTick tick;

    std::cout << tsTime::current().str() << std::endl;

    tsTickPrice priceTick(objID, 23.42);

    try
    {
        tsTickSender sender("localhost");

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

