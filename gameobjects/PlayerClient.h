#pragma once

#include "Player.h"
#include "events/EventManager.h"
#include "graphics/AnimatedSpriteComponent.h"
#include "graphics/AssetManager.h"
#include "graphics/RenderManager.h"

namespace gameobjects
{

const std::string PlayerSheetKey = "player";

class PlayerClient : public Player
{
  public:
    CLASS_IDENTIFICATION(PLAYER_ID)
    MovementOrientation GetCurrentOrientation();
    MovementType GetCurrentMovementType();

    void Update() override;
    virtual void HandleStateMessage(std::shared_ptr<PhysicsComponentUpdate> stateEvent) override;

    // PIMP: real gross static map that maps clients to players. Probably need to so something
    // better
    static inline std::unordered_map<uint32_t, uint32_t> clientToPlayer;

    static std::unique_ptr<SimpleGameObject> StaticCreate()
    {
        auto instance = std::make_unique<PlayerClient>();
        RenderManager::sInstance->AddComponent(instance->drawable.get());
        // RenderManager::sInstance->AddComponent(instance->serverLocation.get());

        return std::move(instance);
    }

    virtual void SetupListeners() override
    {
        Player::SetupListeners();
        // add it's world id and player id here cause I'm bad at programming
        clientToPlayer[clientOwnerId] = GetWorldId();
    }

    PlayerClient() : stateDirty(true), predictedState(std::make_shared<PhysicsComponent>())
    {
        auto playerSheet = AssetManager::sInstance->GetAnimatedSheetData(PlayerSheetKey);

        this->drawable = std::make_shared<AnimatedSpriteComponent<PlayerClient>>(this, playerSheet);

        // TODO: Extract this out to something better
        // this->serverLocation =
        //     std::make_shared<AnimatedSpriteComponent<PhysicsComponent>>(physicsComponent.get(), playerSheet, true);
    }

  private:
    void RemoveOldMoves(std::deque<std::shared_ptr<PlayerInputEvent>>& inMoves);
    void PredictState();
    bool stateDirty;

    // The player has a physics component that holds the state the server
    // thinks the player is. The predicted state is used to predict other player
    // objects locations or the location of the player after the unprocessed
    // moves have been applied.
    std::shared_ptr<PhysicsComponent> predictedState;
    std::shared_ptr<AnimatedSpriteComponent<PlayerClient>> drawable;
    // std::shared_ptr<AnimatedSpriteComponent<PhysicsComponent>> serverLocation;
    MovementOrientation direction;
};

} // namespace gameobjects
