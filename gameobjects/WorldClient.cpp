#include "WorldClient.h"
#include "Registry.h"
#include "events/CreatePlayerOwnedObject.h"
#include "logger/Logger.h"

namespace gameobjects
{

std::unique_ptr<WorldClient> WorldClient::sInstance;
WorldClient::WorldClient() {}
void WorldClient::StaticInit() { sInstance.reset(new WorldClient()); };

void WorldClient::OnStateUpdateMessage(std::shared_ptr<Event> objectStateEvent)
{
    // Takes the state and event and passes it the the game object with the id
    // to be processed later?
    INFO("Got a state update");
    auto castMsg = std::static_pointer_cast<PhysicsComponentUpdate>(objectStateEvent);
    uint32_t worldId = castMsg->worldId;

    if (gameObjById.find(worldId) == gameObjById.end())
    {
        ERROR("No game object with id {}", worldId);
        return;
    }

    // This should be replaced with something like "Networked Object"
    gameObjById[worldId]->HandleStateMessage(castMsg);
}

// void WorldClient::OnAddObject(std::shared_ptr<Event> addGameObjEvent)
// {
//     // Handles adding the game object to the world
//     auto castAddObj = std::static_pointer_cast<CreatePlayerOwnedObject>(addGameObjEvent);

//     INFO("Adding game object");
//     auto obj = Registry::sInstance->CreateGameObject(castAddObj->objType);
//     AddGameObject(obj, castAddObj->worldId);
// }
} // namespace gameobjects