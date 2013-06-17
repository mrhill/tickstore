#include <memory>
#include <iostream>
#include <stdexcept>
#include <babel/mem.h>
#include <babel/file.h>
#include "tsdef.h"
#include "tsThread.h"
#include "tsTickSender.h"
#include "tsTracker.h"
#include "tsNode.h"
#include "tsAuth.h"
#include "tsStore.h"
#include "tsJson.h"
#include <zmq.hpp>

struct TestSendThread : public tsThread
{
    virtual void* run(void*)
    {
        bbU8 pwdHash[32];
        memset(pwdHash, 0, sizeof(pwdHash));
        bbU64 testUID = tsAuth::instance().CreateUser("testsender", pwdHash, tsUserPerm_TickToAll);

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
            auth.setUID(testUID);
            memset(auth.mPwdHash, 0, sizeof(auth.mPwdHash));
            sender << auth;

            for (int i=0; i<100; i++)
            {
                sender << priceTick;
                priceTick.setPrice(i);
                priceTick.setVolume(1<<(i&63));
            }
            msleep(2000);
        }
        catch(std::exception& e)
        {
            std::cout << e.what();
        }

        return 0;
    }
};

static std::string getConfigFile(const char* appFile)
{
    bbCHAR* appPath = NULL;
    bbPathSplit(appFile, &appPath, NULL, NULL);
    std::string configFile = std::string(appPath) + bbDIRDELIM + ".tickschleuderrc";
    bbMemFree(appPath);
    return configFile;
}

int main(int argc, char** argv)
{
    std::string configFile = getConfigFile(argv[0]);
    tsJsonTree config(configFile.c_str());

    zmq::context_t zmq_context(1);

    tsAuthMySQL auth(zmq_context, config.root()["auth"]);

    tsTracker tracker;

    try
    {
        tsNode node(zmq_context, tracker);

        TestSendThread sender;
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

