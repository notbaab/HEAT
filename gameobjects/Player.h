#pragma once

#include <deque>

#include "SimpleGameObject.h"
#include "events/EventRouter.h"

#include "logger/Logger.h"

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
    Player() : physicsComponent(std::make_shared<PhysicsComponent>()), lastMoveSeq(0), moving(false), attacking(false)
    {
    }

    void AddMove(std::shared_ptr<PlayerInputEvent> evt);

    virtual void SetupListeners() override
    {
        INFO("Setting up listener {} ", GetWorldId());
        EventRouter<PlayerInputEvent>::sInstance->AddListener(
            GetWorldId(), [this](std::shared_ptr<PlayerInputEvent> evt) { AddMove(evt); });
    }

    std::shared_ptr<PhysicsComponent> physicsComponent;
    Vector3 GetLocation() { return physicsComponent->centerLocation; }
    Vector3 GetVelocity() { return physicsComponent->speed; }

    virtual std::shared_ptr<PhysicsComponentUpdate> CreateStateEvent() override
    {
        return std::make_shared<PhysicsComponentUpdate>(this->GetWorldId(), lastMoveSeq, this->physicsComponent);
    }

  protected:
    void ApplyMoves(std::shared_ptr<PhysicsComponent> component, std::deque<std::shared_ptr<PlayerInputEvent>>& moves);
    uint32_t lastMoveSeq;
    std::deque<std::shared_ptr<PlayerInputEvent>> moves;

    bool moving, attacking;
};

} // namespace gameobjects
