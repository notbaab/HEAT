#pragma once

#include <memory>

#include "Event.h"

// Event that says user logged in
class LoggedIn : public Event
{
  public:
    CLASS_IDENTIFIER(LoggedIn, 'lgdi')
    EVENT_IDENTIFIER(LoggedIn, 'lgdi')
    SERIALIZER

    LoggedIn(){};
    LoggedIn(uint32_t clientId) : clientId(clientId){};

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(clientId);
        return true;
    }

    uint32_t clientId;
};
