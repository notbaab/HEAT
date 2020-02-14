#pragma once

#include "math/Vector3.h"
#include <cstdint>

// Container for holding physical data about something.
// Not really anything but probably want to add an update to it at some
// point and let it handle the physics stuff
class PhysicsComponent
{
  public:
    uint16_t rotation;
    Vector3 centerLocation;
    Vector3 speed;

    // Probably like a width height etc
};
