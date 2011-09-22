#include "tsStoreMemory.h"
#include "tsTickProc.h"
#include <memory>
#include <vector>
#include <iostream>
#include <stdexcept>

template <class T> struct tsVecManagedPtr : public std::vector<T*>
{
    ~tsVecManagedPtr()
    {
        for(const_reverse_iterator it = rbegin(); it!=rend(); it++)
            delete *it;
    }
};

int main(int argc, char** argv)
{
    int procID = 0;
    try
    {
        std::auto_ptr<tsStore> pTickerStore(tsStore::Create(tsStoreBackend_Memory));
        tsVecManagedPtr<tsTickProc> tickProcessors;

        int port = 2227;
        std::cout << __FUNCTION__ ": listening for connections on port " << port << std::endl;
        tsSocket listenSocket(tsSocketType_TCP);
        listenSocket.listen(port);
        while (true)
        {
            int newSocket = listenSocket.accept();
            tickProcessors.push_back(new tsTickProc(*pTickerStore, newSocket, ++procID));
        }
    }
    catch(std::exception& e)
    {
        std::cout << e.what();
    }

    return 0;
}
   
