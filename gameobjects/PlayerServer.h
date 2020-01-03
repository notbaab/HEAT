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
        // EventManager::sInstance->AddListener(EventListenerFunction eventDelegate, uint32_t
        // eventType)

        return std::move(instance);
    }

    void Update() override;

    PlayerServer() {}
    // ~PlayerServer() {}
};

} // namespace gameobjects
