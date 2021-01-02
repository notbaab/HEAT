#pragma once
#include "packets/Message.h"

class ClientConnectionChallengeResponseMessage : public Message
{
  public:
    CLASS_IDENTIFIER(ClientConnectionChallengeResponseMessage, 'CCRP');
    SERIALIZER;

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(xorSalt, "xorSalt");
        stream.serialize(ddosMinPadding, ddosPaddingSize, "ddosPadding");
        return true;
    }

    ClientConnectionChallengeResponseMessage()
    {
        // sent over an authenticated packet
        ddosMinPadding.reserve(ddosPaddingSize);
        ddosMinPadding.assign(ddosPaddingSize, 0);
    }

    ClientConnectionChallengeResponseMessage(uint64_t xorSalt) : ClientConnectionChallengeResponseMessage()
    {
        this->xorSalt = xorSalt;
    }

    uint64_t xorSalt;

  private:
    std::vector<uint8_t> ddosMinPadding;
    static const uint16_t ddosPaddingSize = 100;
};
