#pragma once

class SocketAddressFactory
{
  public:
    static SocketAddressPtr CreateIPv4FromString(const std::string& inString);
};
