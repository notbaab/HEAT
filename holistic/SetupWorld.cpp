#include "gameobjects/Player.h"
#include "gameobjects/Registry.h"
#include "gameobjects/World.h"

namespace holistic
{

// Function that is called to create the registry with all the
// create functions
void SetupWorld()
{
    gameobjects::World::StaticInit();

    // create registry and add all the creation functions we know about
    gameobjects::Registry::StaticInit(gameobjects::World::StaticAddGameObject);
    gameobjects::Registry::sInstance->RegisterCreationFunction(gameobjects::PLAYER_ID,
                                                               gameobjects::Player::StaticCreate);
}

} // namespace holistic
