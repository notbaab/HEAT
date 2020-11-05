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

    PhysicsComponentUpdate(){};

    // TODO: don't use a pointer here since it gets moved before it's sent
    PhysicsComponentUpdate(uint32_t worldId, uint32_t moveSeq, std::shared_ptr<PhysicsComponent> physicsComponent)
        : worldId(worldId), moveSeq(moveSeq), x(physicsComponent->centerLocation.x),
          y(physicsComponent->centerLocation.y), z(physicsComponent->centerLocation.z),
          dX(physicsComponent->speed.x), dY(physicsComponent->speed.y), dZ(physicsComponent->speed.z),
          rotation(physicsComponent->rotation){};

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(worldId);
        stream.serialize(moveSeq);

        // where is it at
        stream.serialize(x);
        stream.serialize(y);
        stream.serialize(z);

        // how fast is it going
        stream.serialize(dX);
        stream.serialize(dY);
        stream.serialize(dZ);

        // where is it looking?
        stream.serialize(rotation);

        return true;
    }

    uint32_t worldId;

    // last move we processed
    uint32_t moveSeq;
    // position
    int32_t x, y, z;
    // velocities
    int32_t dX, dY, dZ;
    uint16_t rotation;
};
