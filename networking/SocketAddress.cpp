#include "SocketAddress.h"
#include <string>

// std::string SocketAddress::ToString() const
// {
// #if _WIN32
//     const sockaddr_in* s = GetAsSockAddrIn();
//     char destinationBuffer[128];
//     InetNtop(s->sin_family, const_cast<in_addr*>(&s->sin_addr), destinationBuffer,
//              sizeof(destinationBuffer));
//     return StringUtils::Sprintf("%s:%d", destinationBuffer, ntohs(s->sin_port));
// #else
//     // not implement on mac for now...
//     char temp[32];
//     snprintf(temp, 32, "%d", port);
//     return std::string(temp);
// #endif
// }

std::string SocketAddress::ToString() const
{
    uint32_t actualAddress = GetIP4Ref();
    uint32_t actualPort = GetPort();
    const uint8_t a = actualAddress & 0xff;
    const uint8_t b = (actualAddress >> 8) & 0xff;
    const uint8_t c = (actualAddress >> 16) & 0xff;
    const uint8_t d = (actualAddress >> 24) & 0xff;

    // Some song and dance for string formating
    std::string format = "%d.%d.%d.%d:%d";
    size_t size = snprintf(nullptr, 0, format.c_str(), a, b, c, d, actualPort) + 1;
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), a, b, c, d, actualPort);

    return std::string(buf.get(), buf.get() + size - 1); //
}

const uint64_t SocketAddress::GetIPPortKey() const
{
    uint16_t port = GetPort();
    uint32_t ip = GetIP4();
    uint64_t key = port;
    // shift the port to the upper bits
    key = key << 32;
    return key |= ip;
}
