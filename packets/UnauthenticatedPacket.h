#pragma once
#include "ReliableOrderedPacket.h"

class UnauthenticatedPacket : public ReliableOrderedPacket
{
  public:
    IDENTIFIER(UnauthenticatedPacket, 'URPK');
    SERIALIZER;
    uint32_t clientSalt;

    UnauthenticatedPacket(std::shared_ptr<MessageSerializer> factory)
        : ReliableOrderedPacket(factory){};

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(clientSalt);
        ReliableOrderedPacket::Serialize(stream);
        stream.serialize(ddosMinPadding, 1000);
        return true;
    }

  private:
    std::vector<uint8_t> ddosMinPadding;
};
