#pragma once
#include <vector>

#include "SimpleGameObject.h"

class Event;

namespace gameobjects
{

class World
{
  public:
    static void StaticInit();
    static std::unique_ptr<World> sInstance;

    // void OnAddObject(std::shared_ptr<Event> addGameObjEvent);
    void AddGameObject(GameObjectPtr inGameObject);
    static void StaticAddGameObject(GameObjectPtr inGameObject);
    static void PrintInfo();

    void OnAddObject(std::shared_ptr<Event> addGameObjEvent);

    void RemoveGameObject(GameObjectPtr inGameObject);

    void Update();

  private:
    World();
    std::vector<GameObjectPtr> mGameObjects;
};

} // namespace gameobjects
