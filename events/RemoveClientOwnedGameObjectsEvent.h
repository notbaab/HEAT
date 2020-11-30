#pragma once

#include "Event.h"

class RemoveClientOwnedGameObjectsEvent : public Event
{
  public:
    CLASS_IDENTIFIER(RemoveClientOwnedGameObjectsEvent, 'RCOE')
    EVENT_IDENTIFIER(RemoveClientOwnedGameObjectsEvent, 'RCOE')
    SERIALIZER

    RemoveClientOwnedGameObjectsEvent(){};
    RemoveClientOwnedGameObjectsEvent(uint32_t clientGameId) : clientGameId(clientGameId) {}

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(clientGameId);
        return true;
    }

    uint32_t clientGameId;
};
