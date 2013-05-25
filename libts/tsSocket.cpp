#include "tsSocket.h"

#ifndef _WIN32
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#define _inet_ntop inet_ntop
#define _inet_pton inet_pton
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

static bool winsockInitialized = false;
static bool winsockInit()
{
    if (winsockInitialized)
        return true;
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD( 2, 2 );

    int err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 ) {
        return false;
    }
    winsockInitialized = true;
    return true;
}

static const char *_inet_ntop(int af, const void *src, char *dst, socklen_t cnt)
{
    if (af == AF_INET)
    {
        struct sockaddr_in in;
        memset(&in, 0, sizeof(in));
        in.sin_family = AF_INET;
        memcpy(&in.sin_addr, src, sizeof(struct in_addr));
        getnameinfo((struct sockaddr *)&in, sizeof(struct sockaddr_in), dst, cnt, NULL, 0, NI_NUMERICHOST);
        return dst;
    }
    else if (af == AF_INET6)
    {
        struct sockaddr_in6 in;
        memset(&in, 0, sizeof(in));
        in.sin6_family = AF_INET6;
        memcpy(&in.sin6_addr, src, sizeof(struct in_addr6));
        getnameinfo((struct sockaddr *)&in, sizeof(struct sockaddr_in6), dst, cnt, NULL, 0, NI_NUMERICHOST);
        return dst;
    }
    return NULL;
}

static int _inet_pton(int af, const char *src, void *dst)
{
    struct addrinfo hints, *res, *ressave;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = af;

    if (getaddrinfo(src, NULL, &hints, &res) != 0)
    {
        printf("Couldn't resolve host %s\n", src);
        return -1;
    }

    ressave = res;

    while (res)
    {
        memcpy(dst, res->ai_addr, res->ai_addrlen);
        res = res->ai_next;
    }

    freeaddrinfo(ressave);
    return 0;
}

#endif

tsSocket::tsSocket(tsSocketType type) :
    mSocket(-1),
    mType((bbU8)type),
    mState((bbU8)tsSocketState_Unconnected),
    mpAddrInfo(NULL),
    mpAddrBound(NULL)
{
    #if bbOS == bbOS_WIN32
    winsockInit();
    #endif
}

tsSocket::~tsSocket()
{
    close();
}

void tsSocket::close()
{
    if (mSocket!=-1)
    {
        #ifdef _WIN32
        closesocket(mSocket);
        #else
        ::close(mSocket);
        #endif
        mSocket = -1;
    }
    mpAddrBound = NULL;
    freeAddressInfo();
    mState = (bbU8)tsSocketState_Unconnected;
}

void tsSocket::freeAddressInfo()
{
    if (mpAddrInfo)
    {
        ::freeaddrinfo(mpAddrInfo); // free the linked-list
        mpAddrInfo = NULL;
    }
}

void tsSocket::getAddressInfo(const char* pHostName, bbU16 port)
{
    struct addrinfo hints;
    char strPort[8];

    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = (mType == tsSocketType_TCP) ? SOCK_STREAM : SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE ;// fill in my IP for me
    snprintf(strPort, 8, "%d", port);

    freeAddressInfo();

    int status = getaddrinfo(pHostName, strPort, &hints, &mpAddrInfo);
    if (status != 0)
        throw tsSocketException(strprintf("%s: getaddrinfo error: %s", __FUNCTION__, gai_strerror(status)));

/*
    printf("IP addresses for %s:\n", pHostName);

    for(struct addrinfo* p = mpAddrInfo; p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;
        char ipstr[INET6_ADDRSTRLEN];

        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // convert the IP to a string and print it:
        _inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("  %s: %s\n", ipver, ipstr);
    }
*/
}

void tsSocket::connect(const char* pHostName, bbU16 port, tsSocketMode openMode)
{
    close();

    mState = (bbU8)tsSocketState_HostLookup;
    getAddressInfo(pHostName, port);

    mState = (bbU8)tsSocketState_Connecting;
    mSocket = socket(mpAddrInfo->ai_family, mpAddrInfo->ai_socktype, mpAddrInfo->ai_protocol);
    if (mSocket == -1) {
        close();
        throw tsSocketException(strprintf("%s: Error %d creating socket", __FUNCTION__, errno));
    }

    int status = ::connect(mSocket, mpAddrInfo->ai_addr, mpAddrInfo->ai_addrlen);
    if (status == -1) {
        close();
        throw tsSocketException(strprintf("%s: Error %d connecting socket", __FUNCTION__, errno));
    }
    mState = (bbU8)tsSocketState_Connected;
}

void tsSocket::bind(const char* pHostName, bbU16 port)
{
    close();

    mState = (bbU8)tsSocketState_HostLookup;
    getAddressInfo(pHostName, port);

    mState = (bbU8)tsSocketState_Connecting;
    struct addrinfo* p;

    for(p = mpAddrInfo; p != NULL; p = p->ai_next)
    {
        mSocket = ::socket(mpAddrInfo->ai_family, mpAddrInfo->ai_socktype, mpAddrInfo->ai_protocol);
        if (mSocket == -1)
            continue;

        if (::bind(mSocket, p->ai_addr, p->ai_addrlen) == -1)
        {
            ::close(mSocket);
            mSocket = -1;
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        close();
        throw tsSocketException(strprintf("%s: Error %d creating and binding socket", __FUNCTION__, errno));
    }

    mpAddrBound = p;
    mState = (bbU8)tsSocketState_BoundState;
}

std::string tsSocket::nameinfo() const
{
    std::string str;

    if (mpAddrBound)
    {
        char host[64];
        char service[20];

        int flags = NI_NUMERICHOST|NI_NUMERICSERV;
        if (mType == tsSocketType_UDP)
            flags |= NI_DGRAM;

        if (!getnameinfo(mpAddrBound->ai_addr, mpAddrBound->ai_addrlen,
                         host, sizeof(host),
                         service, sizeof(service),
                         flags))
        {
            str = host;
            str += ' ';
            str += service;
        }
    }

    return str;
}

void tsSocket::listen(bbU16 port)
{
    close();
    mState = (bbU8)tsSocketState_HostLookup;
    getAddressInfo(NULL, port);

    mState = (bbU8)tsSocketState_Connecting;
    mSocket = ::socket(mpAddrInfo->ai_family, mpAddrInfo->ai_socktype, mpAddrInfo->ai_protocol);
    if (mSocket == -1) {
        close();
        throw tsSocketException(strprintf("%s: Error %d creating socket", __FUNCTION__, errno));
    }

    int state = ::bind(mSocket, mpAddrInfo->ai_addr, mpAddrInfo->ai_addrlen);
    if (state == -1) {
        close();
        throw tsSocketException(strprintf("%s: Error %d", __FUNCTION__, errno));
    }
    mState = (bbU8)tsSocketState_BoundState;

    state = ::listen(mSocket, 10);
    if (state == -1) {
        close();
        throw tsSocketException(strprintf("%s: Error %d", __FUNCTION__, errno));
    }
    mState = (bbU8)tsSocketState_ListeningState;
}

int tsSocket::accept(tsSocket* pSocket)
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);

    int newSocket = ::accept(mSocket, (struct sockaddr *)&their_addr, &addr_size);
    if (newSocket == -1)
        throw tsSocketException(strprintf("%s: Error %d", __FUNCTION__, errno));

    if (pSocket)
        pSocket->setSocketDescriptor(newSocket);

    return newSocket;
}

void tsSocket::send(const char* pBuf, bbU32 len)
{
    while ((bbS32)len > 0)
    {
        int status = ::send(mSocket, (const char*)pBuf, len, 0);
        if (status == -1)
            throw tsSocketException(strprintf("%s: error %d", __FUNCTION__, errno));
        len -= status;
    }
}

int tsSocket::recv(char* pBuf, bbU32 bufsize, int timeoutMs)
{
    int status;

    bbASSERT((bbS32)bufsize > 0);

    if (timeoutMs > 0) // -1 for blocking, 0 for immediate return
    {
        fd_set readfd;
        struct timeval tv;

        FD_ZERO(&readfd);
        FD_SET(mSocket, &readfd);

        if (timeoutMs)
        {
            tv.tv_sec = timeoutMs >> 10;
            tv.tv_usec = (timeoutMs & 1023) << 10;
        }
        else
        {
            tv.tv_sec = 0;
            tv.tv_usec = 0;
        }

        status = ::select(mSocket+1, &readfd, NULL, NULL, &tv);
        if (status == -1)
            throw tsSocketException(strprintf("%s: error %d on select", __FUNCTION__, errno));
        if (status == 0)
            return 0;
    }

    status = ::recv(mSocket, pBuf, bufsize, MSG_DONTWAIT);
    if ((status == -1) && (errno != EWOULDBLOCK))
        throw tsSocketException(strprintf("%s: error %d on recv", __FUNCTION__, errno));
    return status;
}

void tsSocket::setSocketDescriptor(int socket)
{
    close();
    mSocket = socket;
    mState = (bbU8)tsSocketState_Connected;
}

int tsSocket::peerAddress(std::string& hostName) const
{
    char ipstr[INET6_ADDRSTRLEN];
    int port;

    struct sockaddr_storage name;
    socklen_t nameLen = sizeof(name);
    if (getpeername(mSocket, (struct sockaddr*)&name, &nameLen))
        return -1;

    // deal with both IPv4 and IPv6:
    if (name.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&name;
        port = ntohs(s->sin_port);
        _inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
    } else { // AF_INET6
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&name;
        port = ntohs(s->sin6_port);
        _inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
    }

    hostName = ipstr;
    return port;
}

tsSocketSet::tsSocketSet() : mHighestFD(0), mRdIsSet(0), mWrIsSet(0)
{
    FD_ZERO(&mRdFds);
    FD_ZERO(&mWrFds);
}

void tsSocketSet::addRdFD(int fd)
{
    FD_SET(fd, &mRdFds);
    if (fd >= mHighestFD)
        mHighestFD = fd + 1;
    mRdIsSet = 1;
}

void tsSocketSet::addWrFD(int fd)
{
    FD_SET(fd, &mWrFds);
    if (fd >= mHighestFD)
        mHighestFD = fd + 1;
    mWrIsSet = 1;
}

int tsSocketSet::select(int timeoutUs)
{
    struct timeval timeout;

    if (timeoutUs)
    {
        timeout.tv_sec = timeoutUs / 1000000;
        timeout.tv_usec = timeoutUs - (timeout.tv_sec * 1000000);
    }

    int retval = ::select(mHighestFD,
                          mRdIsSet ? &mRdFds : NULL,
                          mWrIsSet ? &mWrFds : NULL,
                          NULL,
                          timeoutUs ? &timeout : NULL);
    if (retval == -1)
        throw tsSocketException(strprintf("%s: error %d on select", __FUNCTION__, errno));

    return retval;
}

