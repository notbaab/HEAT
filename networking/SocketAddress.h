#pragma once

#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

#include <fcntl.h>
#include <iostream>
#include <stdio.h>

// Added for linux
#include <cstring>
#include <memory>

// TODO: Rewrite to include ipv6
class SocketAddress
{
  public:
    SocketAddress(uint32_t inAddress, uint16_t inPort)
    {
        GetAsSockAddrIn()->sin_family = AF_INET;
        GetIP4Ref() = htonl(inAddress);
        GetAsSockAddrIn()->sin_port = htons(inPort);

        port = inPort;
    }

    SocketAddress(const sockaddr& inSockAddr)
    {
        std::memcpy(&mSockAddr, &inSockAddr, sizeof(sockaddr));
    }

    SocketAddress()
    {
        GetAsSockAddrIn()->sin_family = AF_INET;
        GetIP4Ref() = INADDR_ANY;
        GetAsSockAddrIn()->sin_port = 0;
    }

    bool operator==(const SocketAddress& inOther) const
    {
        return (mSockAddr.sa_family == AF_INET &&
                GetAsSockAddrIn()->sin_port == inOther.GetAsSockAddrIn()->sin_port) &&
               (GetIP4Ref() == inOther.GetIP4Ref());
    }

    size_t GetHash() const
    {
        return (GetIP4Ref()) | ((static_cast<uint32_t>(GetAsSockAddrIn()->sin_port)) << 13) |
               mSockAddr.sa_family;
    }

    uint32_t GetSize() const { return sizeof(sockaddr); }

    const uint64_t GetIPPortKey() const;
    std::string ToString() const;

    uint16_t port;

  private:
    friend class UDPSocket;
    friend class TCPSocket;

    sockaddr mSockAddr;
#if _WIN32
    uint32_t& GetIP4Ref()
    {
        return *reinterpret_cast<uint32_t*>(&GetAsSockAddrIn()->sin_addr.S_un.S_addr);
    }
    const uint32_t& GetIP4Ref() const
    {
        return *reinterpret_cast<const uint32_t*>(&GetAsSockAddrIn()->sin_addr.S_un.S_addr);
    }
#else
    uint32_t& GetIP4Ref() { return GetAsSockAddrIn()->sin_addr.s_addr; }
    const uint32_t& GetIP4Ref() const { return GetAsSockAddrIn()->sin_addr.s_addr; }

    const uint16_t GetPort() const { return ntohs(GetAsSockAddrIn()->sin_port); }
    const uint32_t GetIP4() const { return ntohl(GetAsSockAddrIn()->sin_addr.s_addr); }

#endif

    sockaddr_in* GetAsSockAddrIn() { return reinterpret_cast<sockaddr_in*>(&mSockAddr); }
    const sockaddr_in* GetAsSockAddrIn() const
    {
        return reinterpret_cast<const sockaddr_in*>(&mSockAddr);
    }
};

typedef std::shared_ptr<SocketAddress> SocketAddressPtr;

namespace std
{
template <>
struct hash<SocketAddress>
{
    size_t operator()(const SocketAddress& inAddress) const { return inAddress.GetHash(); }
};
} // namespace std
