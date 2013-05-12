#include "tsTickSender.h"
#include "tsTickFinance.h"
#include "tsStore.h"
#include <memory>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv)
{
    tsTickFactoryFinance factory;
    std::auto_ptr<tsStore> pTickerStore(tsStore::Create(factory, tsStoreBackend_MySQL, "tickstore_testmysql"));

    std::cout << "current time " << tsTime::current().str() << std::endl;

    bbU64 sym = 0xDEADDEADUL; sym = (sym<<32) | 0xF00DF00DUL;
    tsObjID objID(0x00000042, sym);

    tsTick tick(objID);
    tsTickPrice priceTick(objID, 23.42, 0);

    try
    {
        //tsTickSender sender(factory, "localhost");

        bbUINT tickSize;
        char buf[tsTick::SERIALIZEDMAXSIZE];

        tickSize = factory.serializedSize(tick);
        factory.serialize(tick, buf);
        pTickerStore->SaveTick(buf, tickSize);

        tickSize = factory.serializedSize(priceTick);
        factory.serialize(priceTick, buf);
        pTickerStore->SaveTick(buf, tickSize);
    }
    catch(std::exception& e)
    {
        std::cout << e.what();
    }

    return 0;
}

