#pragma once
#include "ReliableOrderedPacket.h"

class AuthenticatedPacket : public ReliableOrderedPacket
{
  public:
    IDENTIFIER(AuthenticatedPacket, 'ARPK');
    SERIALIZER;
    uint32_t expectedSalt;

    AuthenticatedPacket(std::shared_ptr<MessageSerializer> factory)
        : ReliableOrderedPacket(factory){};

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(expectedSalt);
        ReliableOrderedPacket::Serialize(stream);
        stream.serialize(ddosMinPadding, 1000);
        return true;
    }

  private:
    std::vector<uint8_t> ddosMinPadding;
};
