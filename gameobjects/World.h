#pragma once
#include <vector>

#include "SimpleGameObject.h"

namespace gameobjects
{

class World
{
  public:
    static void StaticInit();
    static std::unique_ptr<World> sInstance;

    void AddGameObject(GameObjectPtr inGameObject);
    static void StaticAddGameObject(GameObjectPtr inGameObject);
    static void PrintInfo();

    void RemoveGameObject(GameObjectPtr inGameObject);

    void Update();

  private:
    World();
    std::vector<GameObjectPtr> mGameObjects;
};

} // namespace gameobjects
