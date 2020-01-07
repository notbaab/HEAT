#include "PlayerClient.h"
#include "events/PhysicsComponentUpdate.h"
#include "logger/Logger.h"

namespace gameobjects
{

void PlayerClient::Update() {}

void PlayerClient::HandleStateMessage(std::shared_ptr<PhysicsComponentUpdate> stateEvent)
{
    INFO("Handling state, moving from {}, {}", this->physicsComponent->centerLocation.mX,
         this->physicsComponent->centerLocation.mY);
    this->physicsComponent = stateEvent->physicsComponent;
    INFO("Now at {}, {}", this->physicsComponent->centerLocation.mX,
         this->physicsComponent->centerLocation.mY);
}

} // namespace gameobjects
