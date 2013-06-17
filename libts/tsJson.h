#ifndef tsJson_H_
#define tsJson_H_

#include "tsdef.h"
#include "json.h"

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

#endif

