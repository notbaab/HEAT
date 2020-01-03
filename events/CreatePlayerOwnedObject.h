#pragma once

#include "Event.h"

// An event that indicates an object should be created. Only holds the
// game object id of the object being created
class CreatePlayerOwnedObject : public Event
{
  public:
    CLASS_IDENTIFIER(CreatePlayerOwnedObject, 'CPOO')
    EVENT_IDENTIFIER(CreatePlayerOwnedObject, 'CPOO')
    SERIALIZER

    CreatePlayerOwnedObject(){};
    CreatePlayerOwnedObject(uint32_t playerId, uint32_t gameObjectId)
        : playerId(playerId), gameObjectId(gameObjectId){};

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(playerId);
        stream.serialize(gameObjectId);

        return true;
    }

    uint32_t playerId;
    uint32_t gameObjectId;
};
