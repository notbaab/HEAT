#pragma once

#include <functional>
#include <memory>
#include <stdio.h>
#include <unordered_map>

#include "gameobjects/SimpleGameObject.h"
namespace gameobjects
{

using GameObjectCreationFunc = std::function<std::unique_ptr<SimpleGameObject>()>;
using AddToWorldFunction = std::function<void(GameObjectPtr)>;

class Registry
{
  public:
    static void StaticInit(AddToWorldFunction worldAddFunction);

    // Singleton instance
    static std::unique_ptr<Registry> sInstance;

    void RegisterCreationFunction(uint32_t inCCName, GameObjectCreationFunc inCreationFuntion);

    GameObjectPtr CreateGameObject(uint32_t inCCName);

  private:
    Registry();

    AddToWorldFunction mAddToWorldFunction;

    std::unordered_map<uint32_t, GameObjectCreationFunc> mNameToCreationFunction;
};

} // namespace gameobjects
