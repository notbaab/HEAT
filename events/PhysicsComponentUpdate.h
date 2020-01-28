#pragma once

#include <memory>

#include "Event.h"
#include "gameobjects/PhysicsComponent.h"

// PIMP this is going to get ceated a shit load. Figure out how to not create it a million times

// An event that indicates an object should be created. Only holds the
// game object id of the object being created
class PhysicsComponentUpdate : public Event
{
  public:
    CLASS_IDENTIFIER(PhysicsComponentUpdate, 'PCMU')
    EVENT_IDENTIFIER(PhysicsComponentUpdate, 'PCMU')
    SERIALIZER

    PhysicsComponentUpdate() : physicsComponent(std::make_shared<PhysicsComponent>()){};

    PhysicsComponentUpdate(uint32_t worldId, uint32_t moveSeq,
                           std::shared_ptr<PhysicsComponent> physicsComponent)

        : worldId(worldId), moveSeq(moveSeq), physicsComponent(physicsComponent){};

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(worldId);
        stream.serialize(moveSeq);

        // where is it at
        stream.serialize(physicsComponent->centerLocation.mX);
        stream.serialize(physicsComponent->centerLocation.mY);
        stream.serialize(physicsComponent->centerLocation.mZ);
        stream.serialize(physicsComponent->rotation);

        return true;
    }

    uint32_t worldId;

    // last move we processed
    uint32_t moveSeq;
    std::shared_ptr<PhysicsComponent> physicsComponent;
};
