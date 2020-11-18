#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "events/EventManager.h"
#include "events/PlayerInputEvent.h"
#include "gameobjects/Player.h"
#include "gameobjects/WorldClient.h"
#include "holistic/HNetworkManagerClient.h"
#include "logger/Logger.h"

// Only needs to know how to say moves have been received
class NullNetworkManagerClient : public holistic::HNetworkManagerClient
{
  public:
    static inline std::unique_ptr<NullNetworkManagerClient> sInstance;
    static void StaticInit() { sInstance.reset(new NullNetworkManagerClient()); };
    NullNetworkManagerClient() : HNetworkManagerClient(nullptr){};

    uint32_t GetClientId() override { return clientId; }

    virtual void ProcessMessages() override{};

    virtual void SendOutgoingPackets() override{};

    virtual void DataReceivedCallback(SocketAddress fromAddress, std::unique_ptr<std::vector<uint8_t>> data) override{};

    virtual void StartServerHandshake() override
    {
        clientId = holistic::GenerateSalt();
        assert(gameobjects::WorldClient::sInstance);
        // no handshake, just create the player
        auto createPlayer = gameobjects::WorldClient::sInstance->CreatePlayerCreationEvent(clientId);
        EventManager::sInstance->QueueEvent(createPlayer);

        auto playerInputRouter = CREATE_DELEGATE_LAMBDA(NullNetworkManagerClient::sInstance->HandlePlayerInputEvent);
        EventManager::sInstance->AddListener(playerInputRouter, PlayerInputEvent::EVENT_TYPE);
    };

    void Tick(uint32_t timeStep) override
    {
        assert(gameobjects::WorldClient::sInstance);
        // loop through the game world and create up to date state events.
        // Basically replacing acking on the server
        auto snapShot = gameobjects::WorldClient::sInstance->CreateWorldSnapshot();
        for (auto& evt : snapShot)
        {
            if (evt->GetEventType() == PhysicsComponentUpdate::EVENT_TYPE)
            {
                auto castEvt = std::static_pointer_cast<PhysicsComponentUpdate>(evt);
                if (castEvt->worldId == playerId)
                {
                    castEvt->moveSeq = lastMoveSeq;
                }
            }

            EventManager::sInstance->QueueEvent(evt);
        }
    };

    bool HandlePlayerInputEvent(std::shared_ptr<Event> evt)
    {
        auto castedEvt = std::static_pointer_cast<PlayerInputEvent>(evt);
        // super weird, but reach into the world and set the players last move seen to it
        auto player = gameobjects::WorldClient::sInstance->GetGameObject(castedEvt->worldId);

        if (player)
        {
            auto castedPlayer = std::static_pointer_cast<gameobjects::Player>(player);
            playerId = castedEvt->worldId;
            lastMoveSeq = castedEvt->moveSeq;
            TRACE("Move seq {}", lastMoveSeq);
        }

        return true;
    }

    void QueueMessage(std::shared_ptr<Message> messageToQueue) override{};

  protected:
    uint32_t clientId;
    uint32_t lastMoveSeq;
    uint32_t playerId;
};
