#ifndef tsSOCKET_H
#define tsSOCKET_H

#include <babel/defs.h>
#include <string>
#include <stdexcept>
#include <streamprintf.h>

enum tsSocketType
{
    tsSocketType_TCP,
    tsSocketType_UDP
};

enum tsSocketMode
{
    tsSocketMode_Read = 1,
    tsSocketMode_Write = 2,
    tsSocketMode_ReadWrite = 3
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
    tsSocketType     mType;
    tsSocketState    mState;
    struct addrinfo* mpAddrInfo;
    int              mSocket;

    void getAddressInfo(const char* pHostName, bbU16 port);
public:
    tsSocket(tsSocketType type);
    ~tsSocket();
    void setSocketDescriptor(int socket);
    tsSocketState state() const { return mState; }
    void close();
    void connect(const char* pHostName, bbU16 port, tsSocketMode openMode = tsSocketMode_ReadWrite);
    void listen(bbU16 port);
    int accept(tsSocket* pSocket = NULL);
    void send(const char* pBuf, bbU32 len);

    /** Receive bytes from socket.
        @param pBuf Pointer to buffer
        @param bufsize Size of buffer
        @param timeoutUs Timeout in miliseconds, 0 for immediate return, -1 for blocking
        @return Number of bytes read, 0 if connection was closed, -1 if timeout reached
    */
    int recv(char* pBuf, bbU32 bufsize, int timeoutMs);
    int peerAddress(std::string& s) const;

    std::string peerName() const {
        std::string s;
        int port = peerAddress(s);
        return strprintf("%s:%d", s, port);
    }
};

#endif
