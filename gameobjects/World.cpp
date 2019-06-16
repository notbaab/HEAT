
#include "World.h"

namespace gameobjects
{

std::unique_ptr<World> World::sInstance;

World::World() {}

void World::StaticInit() { sInstance.reset(new World()); };

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
