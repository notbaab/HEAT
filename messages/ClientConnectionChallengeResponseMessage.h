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
        stream.serialize(xorSalt);
        stream.serialize(ddosMinPadding, 1000);
        return true;
    }

    ClientConnectionChallengeResponseMessage(){};
    ClientConnectionChallengeResponseMessage(uint64_t xorSalt) : xorSalt(xorSalt)
    {
        // sent over an authenticated packet
        ddosMinPadding.reserve(1000);
        ddosMinPadding.assign(1000, 0);
    };
    uint64_t xorSalt;

  private:
    std::vector<uint8_t> ddosMinPadding;
};
