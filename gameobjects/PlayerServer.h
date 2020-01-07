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

        // listen for move events
        // EventManager::sInstance->AddListener(EventListenerFunction eventDelegate, uint32_t
        // eventType)
        return std::move(instance);
    }

    void Update() override;
    virtual void HandleStateMessage(std::shared_ptr<PhysicsComponentUpdate> stateEvent) override;

    PlayerServer() : stateDirty(true) {}

  private:
    bool stateDirty;
};

} // namespace gameobjects
