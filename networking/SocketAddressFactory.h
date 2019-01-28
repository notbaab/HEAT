#ifndef SocketAddressFactory_h
#define SocketAddressFactory_h

class SocketAddressFactory
{
  public:
    static SocketAddressPtr CreateIPv4FromString(const std::string& inString);
};

#endif
