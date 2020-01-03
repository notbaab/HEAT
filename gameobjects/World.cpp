#include "World.h"
#include "Registry.h"
#include "events/CreatePlayerOwnedObject.h"

namespace gameobjects
{

std::unique_ptr<World> World::sInstance;

World::World() {}

void World::StaticInit() { sInstance.reset(new World()); };

// void World::OnCreateObject(std::shared_ptr<Event> addGameObjEvent)
// {
//     auto castAddObj = std::static_pointer_cast<CreateGameObject>(addGameObjEvent);
// }

// Adds the object with the given id
void World::OnAddObject(std::shared_ptr<Event> addGameObjEvent)
{
    auto castAddObj = std::static_pointer_cast<CreatePlayerOwnedObject>(addGameObjEvent);

    std::cout << "adding game object" << std::endl;
    auto obj = Registry::sInstance->CreateGameObject(castAddObj->gameObjectId);
    AddGameObject(obj);
    std::cout << "Added" << std::endl;
}

void World::AddGameObject(GameObjectPtr obj) { mGameObjects.push_back(obj); }
void World::StaticAddGameObject(GameObjectPtr obj) { sInstance->AddGameObject(obj); }
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

void World::Update()
{
    for (auto go : mGameObjects)
    {
        go->Update();
    }
}

} // namespace gameobjects
