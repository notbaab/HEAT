#include "packets/Message.h"

class LogoutMessage : public Message
{
  public:
    CLASS_IDENTIFIER(LogoutMessage, 'lgot');
    SERIALIZER;

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(clientId);
        return true;
    }

    LogoutMessage(uint32_t clientId) : clientId(clientId){};
    LogoutMessage(){};

    uint32_t clientId;
};
