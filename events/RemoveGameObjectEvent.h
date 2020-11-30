#pragma once

#include "Event.h"

class RemoveGameObjectEvent : public Event
{
  public:
    CLASS_IDENTIFIER(RemoveGameObjectEvent, 'RGOB')
    EVENT_IDENTIFIER(RemoveGameObjectEvent, 'RGOB')
    SERIALIZER

    RemoveGameObjectEvent(){};
    RemoveGameObjectEvent(uint32_t worldId) : worldId(worldId) {}

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(worldId);
        return true;
    }

    uint32_t worldId;
};
