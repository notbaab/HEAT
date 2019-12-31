#include "packets/Message.h"

class ClientConnectionRequestMessage : public Message
{
  public:
    CLASS_IDENTIFIER(ClientConnectionRequestMessage, 'CCRM');
    SERIALIZER;

    // Message contains no information
    // salt is actually in the unauthenticated packet. I don't like this but
    // the way I've structured this is we can read the packets as the come in
    // and we inflated the unauthenticated packets to prevent ddos amplification
    // attacks. Re evaluate once we have some sort of encryption scheme up
    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(salt);
        return true;
    }

    ClientConnectionRequestMessage(){};
    ClientConnectionRequestMessage(uint64_t salt) : salt(salt){};
    uint64_t salt;
};
