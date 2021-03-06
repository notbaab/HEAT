#pragma once

#include "Player.h"
#include "events/EventManager.h"
#include "graphics/AnimatedSpriteComponent.h"
#include "graphics/AssetManager.h"
#include "graphics/RenderManager.h"

namespace gameobjects
{

const std::string PlayerSheetKey = "player";

class PlayerServerGhost
{
  public:
    PlayerServerGhost(std::shared_ptr<PhysicsComponent> location) : location(location)
    {
        auto playerSheet = AssetManager::sInstance->GetAnimatedSheetData(PlayerSheetKey);
        this->drawable = std::make_shared<AnimatedSpriteComponent<PlayerServerGhost>>(this, playerSheet, true);
    };

    ~PlayerServerGhost() { RenderManager::sInstance->RemoveComponent(drawable.get()); }

    Vector3 GetLocation() { return location->centerLocation; }

    std::shared_ptr<PhysicsComponent> location;
    std::shared_ptr<AnimatedSpriteComponent<PlayerServerGhost>> drawable;

    MovementOrientation GetCurrentOrientation() { return MovementOrientation::NONE; };
    MovementType GetCurrentMovementType() { return MovementType::NONE; };
};

class PlayerClient : public Player
{
  public:
    CLASS_IDENTIFICATION(PLAYER_ID)
    MovementOrientation GetCurrentOrientation();
    MovementType GetCurrentMovementType();

    ~PlayerClient() { RenderManager::sInstance->RemoveComponent(drawable.get()); }

    void Update() override;
    virtual void HandleStateMessage(std::shared_ptr<PhysicsComponentUpdate> stateEvent) override;

    static inline uint32_t localPlayerClientId;
    static inline uint32_t localPlayerId;

    static void UserLoggedIn(std::shared_ptr<Event> evt);

    // PIMP: real gross static map that maps clients to players. Probably need to so something
    // better
    static inline std::unordered_map<uint32_t, uint32_t> clientToPlayer;

    static std::unique_ptr<SimpleGameObject> StaticCreate()
    {
        auto instance = std::make_unique<PlayerClient>();
        RenderManager::sInstance->AddComponent(instance->drawable.get());
        RenderManager::sInstance->AddComponent(instance->serverGhost->drawable.get());

        return std::move(instance);
    }

    virtual void AddedToGameWorld() override
    {
        Player::AddedToGameWorld();
        clientToPlayer[clientOwnerId] = GetWorldId();
        if (localPlayerClientId == clientOwnerId)
        {
            localPlayerId = GetWorldId();
        }
    }

    PlayerClient() : stateDirty(true), predictedState(std::make_shared<PhysicsComponent>())
    {
        auto playerSheet = AssetManager::sInstance->GetAnimatedSheetData(PlayerSheetKey);

        this->drawable = std::make_shared<AnimatedSpriteComponent<PlayerClient>>(this, playerSheet);
        this->serverGhost = std::make_unique<PlayerServerGhost>(this->physicsComponent);
    }

    // Use the predicted state
    virtual std::shared_ptr<PhysicsComponentUpdate> CreateStateEvent() override
    {
        return std::make_shared<PhysicsComponentUpdate>(this->GetWorldId(), lastMoveSeq, this->predictedState);
    }

    // Return the predicted state of the client instead of it's true location
    Vector3 GetLocation() { return predictedState->centerLocation; }
    Vector3 GetVelocity() { return predictedState->speed; }

    bool IsLocalPlayer() { return clientOwnerId == localPlayerClientId; };
    void UpdateLocalPlayer();
    void UpdateRemotePlayer();

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

    std::unique_ptr<PlayerServerGhost> serverGhost;

    // The last move we predicted to last frame. If it matches where we
    // ended the prediction, we are no longer moving
    uint32_t lastPredictedMoveSeq;
    MovementOrientation currentOrientation;
};

} // namespace gameobjects
