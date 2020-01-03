#include "Registry.h"

namespace gameobjects
{

std::unique_ptr<Registry> Registry::sInstance;

void Registry::StaticInit(AddToWorldFunction inAddToWorld)
{
    // Reset pointer to new instance of game object registry
    sInstance.reset(new Registry());
    sInstance->mAddToWorldFunction = inAddToWorld;
}

Registry::Registry() {}

std::shared_ptr<SimpleGameObject> Registry::CreateGameObject(uint32_t inCCName)
{
    GameObjectPtr newObject = sInstance->mNameToCreationFunction[inCCName]();

    // sInstance->mAddToWorldFunction(newObject);

    return newObject;
}

void Registry::RegisterCreationFunction(uint32_t inCCName, GameObjectCreationFunc inCreationFuntion)
{
    sInstance->mNameToCreationFunction[inCCName] = inCreationFuntion;
}

} // namespace gameobjects
