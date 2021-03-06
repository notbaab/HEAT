#pragma once
#include "ReliableOrderedPacket.h"

class AuthenticatedPacket : public ReliableOrderedPacket
{
  public:
    CLASS_IDENTIFIER(AuthenticatedPacket, 'ARPK')
    SERIALIZER
    uint32_t expectedSalt;

    AuthenticatedPacket(std::shared_ptr<MessageSerializer> factory) : ReliableOrderedPacket(factory) {}

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(expectedSalt, "expectedSalt");
        ReliableOrderedPacket::Serialize(stream);
        return true;
    }
};
