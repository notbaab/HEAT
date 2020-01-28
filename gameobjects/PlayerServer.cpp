#include "PlayerServer.h"
#include "events/PhysicsComponentUpdate.h"
#include "logger/Logger.h"

namespace gameobjects
{

void PlayerServer::Update()
{
    if (stateDirty)
    {
        // send an update message. For now it's just the physics component that is
        // being updated.
        auto phyEvt = std::make_shared<PhysicsComponentUpdate>(this->GetWorldId(), lastMoveSeq,
                                                               this->physicsComponent);
        EventManager::sInstance->QueueEvent(phyEvt);
        stateDirty = false;
    }
}

void PlayerServer::HandleStateMessage(std::shared_ptr<PhysicsComponentUpdate> stateEvent) {}

} // namespace gameobjects
