#pragma once

#include <cstdint>
#include <string>

#include "math/Vector3.h"

// Container for holding physical data about something.
// Not really anything but probably want to add an update to it at some
// point and let it handle the physics stuff
class PhysicsComponent
{
  public:
    uint16_t rotation;
    Vector3 centerLocation;
    Vector3 speed;

    MovementOrientation GetCurrentOrientation() { return OrientationFromVector(speed); }
    bool IsMoving() { return speed.x != 0 || speed.y != 0 || speed.z != 0; }
};
