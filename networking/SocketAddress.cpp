#include "SocketAddress.h"

std::string SocketAddress::ToString() const
{
#if _WIN32
    const sockaddr_in* s = GetAsSockAddrIn();
    char destinationBuffer[128];
    InetNtop(s->sin_family, const_cast<in_addr*>(&s->sin_addr), destinationBuffer,
             sizeof(destinationBuffer));
    return StringUtils::Sprintf("%s:%d", destinationBuffer, ntohs(s->sin_port));
#else
    // not implement on mac for now...
    char temp[32];
    snprintf(temp, 32, "%d", port);
    return std::string(temp);
#endif
}
