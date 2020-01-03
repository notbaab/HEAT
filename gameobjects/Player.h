#pragma once

#include "SimpleGameObject.h"

namespace gameobjects
{

// PlayerServer and client have the same id since the id is used to
// identify the class and for the server it's the PlayerServer class and
// for the client it's the player client class
const uint32_t PLAYER_ID = 0x13129328;

class Player : public SimpleGameObject
{
};

} // namespace gameobjects
