#include "World.h"
#include "Player.h"
#include "Registry.h"
#include "events/CreatePlayerOwnedObject.h"
#include "logger/Logger.h"

namespace gameobjects
{

std::unique_ptr<World> World::sInstance;

World::World() { nextGameObjectId = 1; }

void World::StaticInit() { sInstance.reset(new World()); };

// void World::OnCreateObject(std::shared_ptr<Event> addGameObjEvent)
// {
//     auto castAddObj = std::static_pointer_cast<CreateGameObject>(addGameObjEvent);
// }

// Adds the object with the given id
void World::OnAddObject(std::shared_ptr<Event> addGameObjEvent)
{
    auto castAddObj = std::static_pointer_cast<CreatePlayerOwnedObject>(addGameObjEvent);

    INFO("Adding game object {} with id {}", castAddObj->objType, castAddObj->worldId);
    auto obj = Registry::sInstance->CreateGameObject(castAddObj->objType);
    AddGameObject(obj, castAddObj->worldId);
}

// Kinda the factory function for creating object creation events?
std::shared_ptr<CreatePlayerOwnedObject> World::CreatePlayerCreationEvent(uint32_t playerId)
{
    auto createPlayer =
        std::make_shared<CreatePlayerOwnedObject>(playerId, PLAYER_ID, nextGameObjectId);
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
    return true;
}

bool World::StaticAddGameObject(GameObjectPtr obj, uint32_t worldId)
{
    return sInstance->AddGameObject(obj, worldId);
}

void World::PrintInfo()
{
    for (auto go : sInstance->mGameObjects)
    {
        // TODO: Print Info delegate to game object
    }
}

void World::RemoveGameObject(GameObjectPtr obj)
{
    for (auto go : mGameObjects)
    {
    }
}

void World::Update(uint32_t currentTime)
{
    for (auto go : mGameObjects)
    {
        go->Update();
    }
}

} // namespace gameobjects
