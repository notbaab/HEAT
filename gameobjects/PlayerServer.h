#pragma once

#include "Player.h"
#include "events/EventManager.h"

namespace gameobjects
{

class PlayerServer : public Player
{
  public:
    CLASS_IDENTIFICATION(PLAYER_ID)
    static std::unique_ptr<SimpleGameObject> StaticCreate()
    {
        auto instance = std::make_unique<PlayerServer>();
        return std::move(instance);
    }

    void Update() override;
    virtual void HandleStateMessage(std::shared_ptr<PhysicsComponentUpdate> stateEvent) override;
    std::shared_ptr<PhysicsComponentUpdate> CreateStateUpdateEvent();

    PlayerServer() : stateDirty(true) {}

  private:
    bool stateDirty;
};

} // namespace gameobjects
