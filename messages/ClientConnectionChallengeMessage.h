#include "packets/Message.h"

class ClientConnectionChallengeMessage : public Message
{
  public:
    IDENTIFIER(ClientConnectionChallengeMessage, 'CCCM');
    SERIALIZER;

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(clientSalt);
        stream.serialize(serverSalt);
        return true;
    }

    ClientConnectionChallengeMessage(uint64_t clientSalt, uint64_t serverSalt)
        : clientSalt(clientSalt), serverSalt(serverSalt){};
    ClientConnectionChallengeMessage(){};

    uint64_t clientSalt;
    uint64_t serverSalt;
};
