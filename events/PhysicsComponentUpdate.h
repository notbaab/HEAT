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
        : worldId(worldId), moveSeq(moveSeq), x(physicsComponent->centerLocation.mX),
          y(physicsComponent->centerLocation.mY), z(physicsComponent->centerLocation.mZ),
          dX(physicsComponent->speed.mX), dY(physicsComponent->speed.mY), dZ(physicsComponent->speed.mZ),
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
    uint32_t x, y, z;
    // velocities
    uint32_t dX, dY, dZ;
    uint16_t rotation;
};
