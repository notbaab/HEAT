#pragma once
#include "packets/Message.h"

class ClientLoginMessage : public Message
{
  public:
    CLASS_IDENTIFIER(ClientLoginMessage, 'LOGI')
    SERIALIZER

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(userName);
        return true;
    }

    ClientLoginMessage(){};
    ClientLoginMessage(std::string userName) : userName(userName){};
    std::string userName;
};
