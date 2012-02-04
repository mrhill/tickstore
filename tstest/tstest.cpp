#include "tsTickSender.h"
#include "tsTickFinance.h"
#include <memory>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv)
{
    tsTickFactoryFinance factory;

    bbU64 sym = 0xDEADDEADUL;
    sym = (sym<<32) | 0xF00DF00DUL;
    tsObjID objID(0xE400002E, sym);
    tsTick tick;

    std::cout << tsTime::current().str() << std::endl;

    tsTickPrice priceTick(objID, 23.42, 0);

    try
    {
        tsTickSender sender(factory, "localhost");

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

