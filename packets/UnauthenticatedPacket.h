#pragma once
#include "ReliableOrderedPacket.h"

class UnauthenticatedPacket : public ReliableOrderedPacket
{
  public:
    CLASS_IDENTIFIER(UnauthenticatedPacket, 'URPK');
    SERIALIZER;

    UnauthenticatedPacket(std::shared_ptr<MessageSerializer> factory) : ReliableOrderedPacket(factory)
    {
        ddosMinPadding.reserve(ddosPaddingSize);
        ddosMinPadding.assign(ddosPaddingSize, 0);
    };

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        ReliableOrderedPacket::Serialize(stream);
        stream.serialize(ddosMinPadding, ddosPaddingSize, "ddosPadding");
        return true;
    }

  private:
    std::vector<uint8_t> ddosMinPadding;
    static const uint16_t ddosPaddingSize = 1000;
};
