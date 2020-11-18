#pragma once

#include <unordered_map>
#include <vector>

#include "SimpleGameObject.h"

class Event;
class CreatePlayerOwnedObject;

namespace gameobjects
{

class World
{
  public:
    static void StaticInit();
    static std::unique_ptr<World> sInstance;

    // Debug stuff
    static std::string DebugWorld(std::vector<std::string> args);

    // void OnAddObject(std::shared_ptr<Event> addGameObjEvent);
    bool AddGameObject(GameObjectPtr inGameObject, uint32_t worldId);
    static bool StaticAddGameObject(GameObjectPtr inGameObject, uint32_t worldId);
    static void PrintInfo();

    virtual void OnAddObject(std::shared_ptr<Event> addGameObjEvent);

    void RemoveGameObject(GameObjectPtr inGameObject);
    void Update(uint32_t currentTime);
    GameObjectPtr GetGameObject(uint32_t gameObjectId);
    std::shared_ptr<CreatePlayerOwnedObject> CreatePlayerCreationEvent(uint32_t playerId);
    std::vector<std::shared_ptr<Event>> CreateWelcomeStateEvents();
    std::vector<std::shared_ptr<Event>> CreateWorldSnapshot();

  protected:
    World();

    uint32_t nextGameObjectId;

    std::vector<GameObjectPtr> mGameObjects;
    std::unordered_map<uint32_t, GameObjectPtr> gameObjById;
};

} // namespace gameobjects
