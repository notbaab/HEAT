#pragma once

#include "Player.h"
#include "events/EventManager.h"
#include "graphics/AnimatedSpriteComponent.h"
#include "graphics/RenderManager.h"

namespace gameobjects
{

const std::string PlayerSheetKey = "player";

class PlayerClient : public Player
{
  public:
    CLASS_IDENTIFICATION(PLAYER_ID)
    // PIMP: real gross static map that maps clients to players. Probably need to so something
    // better
    static inline std::unordered_map<uint32_t, uint32_t> clientToPlayer;

    static std::unique_ptr<SimpleGameObject> StaticCreate()
    {
        auto instance = std::make_unique<PlayerClient>();

        // listen for move events
        // EventManager::sInstance->AddListener(EventListenerFunction eventDelegate, uint32_t
        // eventType)

        RenderManager::sInstance->AddComponent(instance->drawable.get());

        return std::move(instance);
    }

    void Update() override;

    virtual void HandleStateMessage(std::shared_ptr<PhysicsComponentUpdate> stateEvent) override;

    virtual void SetupListeners() override
    {
        Player::SetupListeners();
        // add it's world id and player id here cause I'm bad at programming
        clientToPlayer[clientOwnerId] = GetWorldId();
    }

    PlayerClient() : stateDirty(true)
    {
        auto playerSheet = SpriteSheetData::GetSheetData(PlayerSheetKey);
        this->drawable = std::make_shared<AnimatedSpriteComponent<PlayerClient>>(this, playerSheet);
        // TODO: wut? This is stupid, do more better
        this->drawable->ChangeAnimation("first");
    }

  private:
    bool stateDirty;
    // The player has a physics component that holds the state the server
    // thinks the player is. The predicted state is used to predict other player
    // objects locations or the location of the player after the unprocessed
    // moves have been applied.
    std::shared_ptr<PhysicsComponent> predictedState;
    std::shared_ptr<AnimatedSpriteComponent<PlayerClient>> drawable;
};

} // namespace gameobjects
