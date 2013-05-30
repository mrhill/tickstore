#include "tsStore.h"
#include "tsTickClient.h"
#include "tsTickReceiver.h"
#include "tsTickSender.h"
#include <memory>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv)
{
    std::auto_ptr<tsStore> pTickerStore(tsStore::Create(tsStoreBackend_MySQL, "ticks"));
    tsTickClient node("tickstore", "localhost");

    tsTickSubscribe subcribe;
    //sender << subcribe;

    return 0;
}

