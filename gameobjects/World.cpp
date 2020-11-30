#include "World.h"
#include "Player.h"
#include "Registry.h"
#include "events/CreatePlayerOwnedObject.h"
#include "events/RemoveClientOwnedGameObjectsEvent.h"
#include "events/RemoveGameObjectEvent.h"
#include "logger/Logger.h"

namespace gameobjects
{

std::unique_ptr<World> World::sInstance;

World::World() { nextGameObjectId = 1; }

void World::StaticInit() { sInstance.reset(new World()); };

// Go through the game objects and generate events for things that new players
// need to know about. Should just be player creation events?
// PIMP: Keep all the needed events for login in a buffer somewhere
std::vector<std::shared_ptr<Event>> World::CreateWelcomeStateEvents()
{
    std::vector<std::shared_ptr<Event>> events;
    for (auto go : mGameObjects)
    {
        // Get all player control objects
        if (go->GetClassId() == PLAYER_ID)
        {
            auto createPlayerEvent =
                std::make_shared<CreatePlayerOwnedObject>(go->clientOwnerId, PLAYER_ID, go->GetWorldId());
            events.push_back(createPlayerEvent);
        }
    }
    return events;
}

std::vector<std::shared_ptr<Event>> World::CreateWorldSnapshot()
{
    std::vector<std::shared_ptr<Event>> events;
    for (auto go : mGameObjects)
    {
        auto worldState = go->CreateStateEvent();
        events.push_back(worldState);
    }

    return events;
}

// Adds the object with the given id
void World::OnAddObject(std::shared_ptr<Event> addGameObjEvent)
{
    auto castAddObj = std::static_pointer_cast<CreatePlayerOwnedObject>(addGameObjEvent);

    INFO("Adding game object type {} with id {}", castAddObj->objType, castAddObj->worldId);
    auto obj = Registry::sInstance->CreateGameObject(castAddObj->objType);
    obj->clientOwnerId = castAddObj->playerId;
    AddGameObject(obj, castAddObj->worldId);
}

void World::OnRemoveClientOwnedObjects(std::shared_ptr<Event> removeClientObject)
{
    auto castRemoveObj = std::static_pointer_cast<RemoveClientOwnedGameObjectsEvent>(removeClientObject);

    INFO("Removing game object with client id {}", castRemoveObj->clientGameId);
    RemoveClientOwnedGameObjects(castRemoveObj->clientGameId);
}

// Adds the object with the given id
void World::OnRemoveObject(std::shared_ptr<Event> removeGameObject)
{
    auto castRemoveObj = std::static_pointer_cast<RemoveGameObjectEvent>(removeGameObject);

    INFO("Removing game object with id {}", castRemoveObj->worldId);
    RemoveGameObject(castRemoveObj->worldId);
}

// TODO: This is only ever called on the server which is correct, but maybe factor out world server into it's own thing
// Kinda the factory function for creating object creation events?
std::shared_ptr<CreatePlayerOwnedObject> World::CreatePlayerCreationEvent(uint32_t playerId)
{
    auto createPlayer = std::make_shared<CreatePlayerOwnedObject>(playerId, PLAYER_ID, nextGameObjectId);
    nextGameObjectId++;
    return createPlayer;
}

// Not thread safe. We should maybe try to make it thread safe?
bool World::AddGameObject(GameObjectPtr obj, uint32_t worldId)
{
    obj->SetWorldId(worldId);
    if (gameObjById.find(worldId) != gameObjById.end())
    {
        ERROR("Adding a object with an id that already exists!");
        return false;
    }

    mGameObjects.push_back(obj);
    gameObjById[worldId] = obj;

    obj->AddedToGameWorld();

    return true;
}

bool World::StaticAddGameObject(GameObjectPtr obj, uint32_t worldId) { return sInstance->AddGameObject(obj, worldId); }

std::string World::DebugWorld(std::vector<std::string> args)
{
    for (auto go : sInstance->mGameObjects)
    {
        // TODO: Print Info delegate to game object
    }
    return "did a thing";
}

void World::PrintInfo()
{
    for (auto go : sInstance->mGameObjects)
    {
        // TODO: Print Info delegate to game object
    }
}

GameObjectPtr World::GetGameObject(uint32_t gameObjectId)
{
    for (auto go : mGameObjects)
    {
        if (go->GetWorldId() == gameObjectId)
        {
            return go;
        }
    }
    return nullptr;
}

void World::RemoveClientOwnedGameObjects(uint32_t clientId)
{
    INFO("Removing Client {} Objects", clientId);
    for (auto gameObj = mGameObjects.begin(); gameObj != mGameObjects.end();)
    {
        if (gameObj->get()->clientOwnerId == clientId)
        {
            gameObjById.erase(gameObj->get()->GetWorldId());
            gameObj = mGameObjects.erase(gameObj); // reseat iterator to a valid value post-erase
        }
        else
        {
            ++gameObj;
        }
    }
}

bool World::RemoveGameObject(uint32_t gameObjId)
{
    if (gameObjById.count(gameObjId) == 0)
    {
        ERROR("No game object found with id {}", gameObjId);
        return false;
    }

    for (auto gameObj = mGameObjects.begin(); gameObj != mGameObjects.end();)
    {
        if (gameObj->get()->GetWorldId() == gameObjId)
        {
            gameObj = mGameObjects.erase(gameObj); // reseat iterator to a valid value post-erase
        }
        else
        {
            ++gameObj;
        }
    }
    gameObjById.erase(gameObjId);
    return true;
}

bool World::RemoveGameObject(GameObjectPtr obj) { return RemoveGameObject(obj->GetWorldId()); }

void World::Update(uint32_t currentTime)
{
    for (auto go : mGameObjects)
    {
        go->Update();
    }
}

} // namespace gameobjects
