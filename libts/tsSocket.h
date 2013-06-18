#ifndef tsSOCKET_H
#define tsSOCKET_H

#include <babel/defs.h>
#include "tsdef.h"
#include <string>
#include <stdexcept>

enum tsSocketType
{
    tsSocketType_TCP,
    tsSocketType_UDP
};

enum tsSocketState
{
    tsSocketState_Unconnected,
    tsSocketState_HostLookup,
    tsSocketState_Connecting,
    tsSocketState_Connected,
    tsSocketState_BoundState,
    tsSocketState_ClosingState,
    tsSocketState_ListeningState,
};

struct tsSocketListener
{
    virtual void onSocketConnected() {}
    virtual void onSocketDisconnected() {}

};

class tsSocketException : public std::runtime_error
{
public:
    explicit tsSocketException(const std::string& msg) : std::runtime_error(msg) {}
};

struct addrinfo;

class tsSocket
{
    int              mSocket;   //!< Socket FD
    tsSocketState    mState;
    bbU8             mType;     //!< tsSocketType
    bbU8             mNonBlocking;
    struct addrinfo* mpAddrInfo;
    struct addrinfo* mpAddrBound;

    void freeAddressInfo();
    void getAddressInfo(const char* pHostName, bbU16 port);
public:
    tsSocket(tsSocketType type);
    ~tsSocket();
    void attachFD(int socket);
    int detachFD();
    inline tsSocketState state() const { return (tsSocketState)mState; }
    inline int fd() const { return mSocket; }
    void close();

    void prepare(const char* pHostName, bbU16 port);
    void connect();

    void connect(const char* pHostName, bbU16 port);

    /** Bind socket to local host.
        @param pHostName hostname or NULL local loopback
        @param Port number
    */
    void bind(const char* pHostName, bbU16 port);

    std::string nameinfo() const;

    void listen(bbU16 port, bbUINT backlog = 16);
    int accept(tsSocket* pSocket = NULL);

    /** Send bytes to socket.
        @param pBuf Pointer to buffer
        @param len Number of bytes to send
        @param timeoutMs Timeout in miliseconds, 0 for immediate return, -1 for blocking
    */
    int send(const char* pBuf, bbU32 len, int timeoutMs = -1);

    /** Receive bytes from socket.
        @param pBuf Pointer to buffer
        @param bufsize Size of buffer
        @param timeoutMs Timeout in miliseconds, 0 for immediate return, -1 for blocking
        @return Number of bytes read, 0 if connection was closed, -1 if timeout reached
    */
    int recv(char* pBuf, bbU32 bufsize, int timeoutMs);
    int peerAddress(std::string& s) const;

    std::string peerName() const {
        std::string s;
        int port = peerAddress(s);
        return strprintf("%s:%d", s.c_str(), port);
    }

    static int setFDNonBlocking(int fd);
    inline int setNonBlocking() { mNonBlocking=1; return setFDNonBlocking(mSocket); }
};

class tsSocketSet
{
    fd_set mRdFds;
    fd_set mWrFds;
    int    mHighestFD;
    bbU8   mRdIsSet;
    bbU8   mWrIsSet;
public:
    tsSocketSet();
    void addRdFD(int fd);
    void addWrFD(int fd);
    bool testRdFD(int fd) { return FD_ISSET(fd, &mRdFds); }
    bool testWrFD(int fd) { return FD_ISSET(fd, &mWrFds); }
    int select(int timeoutUs = 0);

    static const unsigned MAXSETSIZE = FD_SETSIZE;
};

#endif
