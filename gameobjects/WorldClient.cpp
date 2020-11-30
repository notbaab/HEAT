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
    auto castMsg = std::static_pointer_cast<PhysicsComponentUpdate>(objectStateEvent);
    uint32_t worldId = castMsg->worldId;

    if (gameObjById.find(worldId) == gameObjById.end())
    {
        ERROR("No game object with id {}", worldId);
        return;
    }
    TRACE("Got a state update for id {}", worldId);

    // This should be replaced with something like "Networked Object"
    gameObjById[worldId]->HandleStateMessage(castMsg);
}

void WorldClient::OnRemoveClientOwnedObjects(std::shared_ptr<Event> removeClientObject)
{
    World::OnRemoveClientOwnedObjects(removeClientObject);
}
} // namespace gameobjects
