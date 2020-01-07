#include "events/CreatePlayerOwnedObject.h"
#include "events/EventManager.h"
#include "events/PhysicsComponentUpdate.h"
#include "gameobjects/PlayerServer.h"
#include "gameobjects/Registry.h"
#include "gameobjects/World.h"
#include "managers/NetworkManagerServer.h"

namespace holistic
{

// Function that is called to create the registry with all the
// create functions
void SetupWorld()
{
    EventManager::StaticInit();

    gameobjects::World::StaticInit();

    // create registry and add all the creation functions we know about
    gameobjects::Registry::StaticInit(gameobjects::World::StaticAddGameObject);
    gameobjects::Registry::sInstance->RegisterCreationFunction(
        gameobjects::PLAYER_ID, gameobjects::PlayerServer::StaticCreate);

    // Event forwarder takes events and pushes them to clients
    auto evtForwarder =
        CREATE_DELEGATE(&NetworkManagerServer::EventForwarder, NetworkManagerServer::sInstance);
    EventManager::sInstance->AddListener(evtForwarder, CreatePlayerOwnedObject::EVENT_TYPE);
    EventManager::sInstance->AddListener(evtForwarder, PhysicsComponentUpdate::EVENT_TYPE);

    // World listens for requests to add objects
    auto addObject =
        CREATE_DELEGATE(&gameobjects::World::OnAddObject, gameobjects::World::sInstance);
    EventManager::sInstance->AddListener(addObject, CreatePlayerOwnedObject::EVENT_TYPE);
}

} // namespace holistic
