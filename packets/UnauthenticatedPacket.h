#pragma once
#include "ReliableOrderedPacket.h"

class UnauthenticatedPacket : public ReliableOrderedPacket
{
  public:
    CLASS_IDENTIFIER(UnauthenticatedPacket, 'URPK');
    SERIALIZER;

    UnauthenticatedPacket(std::shared_ptr<MessageSerializer> factory)
        : ReliableOrderedPacket(factory)
    {
        ddosMinPadding.reserve(1000);
        ddosMinPadding.assign(1000, 0);
    };

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        ReliableOrderedPacket::Serialize(stream);
        stream.serialize(ddosMinPadding, 1000);
        return true;
    }

  private:
    std::vector<uint8_t> ddosMinPadding;
};
