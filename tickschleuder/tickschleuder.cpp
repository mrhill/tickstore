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
#include "json.h"

struct TestSendThread : public tsThread
{
    bbU64 mTestUID;

    TestSendThread(tsStore* pStore)
    {
        bbU8 pwdHash[32];
        memset(pwdHash, 0, sizeof(pwdHash));
        mTestUID = tsAuth::instance().CreateUser("testsender", pwdHash, tsUserPerm_TickToAll);
    }

    virtual void* run()
    {
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

class tsJsonTree
{
    json_value* mpRoot;
public:
    const json_value& root() const { return *mpRoot; }

    tsJsonTree(json_value* root = NULL) : mpRoot(root) {}

    tsJsonTree(const char* jsonFile) : mpRoot(NULL)
    {
        FILE* fh = fopen(jsonFile, "rb");
        if (!fh)
            throw std::runtime_error(strprintf("cannot open file %s", jsonFile));

        fseek(fh, 0, SEEK_END);
        long fileSize = ftell(fh);
        fseek(fh, 0, SEEK_SET);

        char* pData = (char*)malloc(fileSize);
        if (!pData)
        {
            fclose(fh);
            throw std::runtime_error(strprintf("cannot allocate %u bytes to %s", fileSize, jsonFile));
        }

        size_t numRead = fread(pData, fileSize, 1, fh);
        fclose(fh);
        if (numRead != 1)
        {
            free(pData);
            throw std::runtime_error(strprintf("error reading config %s", jsonFile));
        }

        json_settings settings = {0};
        char error[256] = {0};
        mpRoot = json_parse_ex(&settings, pData, fileSize, error);
        bbMemFree(pData);
        if (!mpRoot)
            throw std::runtime_error(strprintf("error parsing config %s: %s", jsonFile, error));
    }

    ~tsJsonTree() { json_value_free(mpRoot); }
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

    tsAuthMySQL auth(config.root()["auth"]);

    tsTracker tracker;

    try
    {
        std::auto_ptr<tsStore> pTickerStore(tsStore::Create(tsStoreBackend_MySQL, "ticks"));

        tsNode node(tracker);

        TestSendThread sender(pTickerStore.get());
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

