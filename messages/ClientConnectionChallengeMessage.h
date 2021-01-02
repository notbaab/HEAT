#include "packets/Message.h"

class ClientConnectionChallengeMessage : public Message
{
  public:
    CLASS_IDENTIFIER(ClientConnectionChallengeMessage, 'CCCM');
    SERIALIZER;

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(serverSalt, "serverSalt");
        return true;
    }

    ClientConnectionChallengeMessage(uint64_t serverSalt) : serverSalt(serverSalt){};
    ClientConnectionChallengeMessage(){};

    uint64_t serverSalt;
};
