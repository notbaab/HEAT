#pragma once

#include "SimpleGameObject.h"

class PhysicsComponent;
class PlayerInputEvent;

namespace gameobjects
{

// PlayerServer and client have the same id since the id is used to
// identify the class and for the server it's the PlayerServer class and
// for the client it's the player client class
const uint32_t PLAYER_ID = 0x13129328;

class Player : public SimpleGameObject
{
  public:
    Player() : physicsComponent(std::make_shared<PhysicsComponent>()), lastMoveSeq(0) {}
    void HandleInput(std::shared_ptr<PlayerInputEvent> evt);
    void Update();

    std::shared_ptr<PhysicsComponent> physicsComponent;

  protected:
    uint32_t lastMoveSeq;
};

} // namespace gameobjects
