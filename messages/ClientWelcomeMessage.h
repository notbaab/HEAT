#pragma once

#include "packets/Message.h"

class ClientWelcomeMessage : public Message
{
  public:
    std::string name;
    CLASS_IDENTIFIER(ClientWelcomeMessage, 'WLCM');
    SERIALIZER;

    ClientWelcomeMessage(){};
    ClientWelcomeMessage(std::string name) : name(name){};

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(name);
        return true;
    }
};
