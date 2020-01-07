#pragma once

#include "Player.h"
#include "events/EventManager.h"
#include "graphics/DrawableComponent.h"

namespace gameobjects
{

class PlayerClient : public Player
{
  public:
    CLASS_IDENTIFICATION(PLAYER_ID)
    static std::unique_ptr<SimpleGameObject> StaticCreate()
    {
        auto instance = std::make_unique<PlayerClient>();

        // listen for move events
        // EventManager::sInstance->AddListener(EventListenerFunction eventDelegate, uint32_t
        // eventType)

        return std::move(instance);
    }

    void Update() override;

    virtual void HandleStateMessage(std::shared_ptr<PhysicsComponentUpdate> stateEvent) override;
    PlayerClient() : stateDirty(true) {}

  private:
    bool stateDirty;
    // The player has a physics component that holds the state the server
    // thinks the player is. The predicted state is used to predict other player
    // objects locations or the location of the player after the unprocessed
    // moves have been applied.
    std::shared_ptr<PhysicsComponent> predictedState;
    std::shared_ptr<DrawableComponent> drawable;
};

} // namespace gameobjects
