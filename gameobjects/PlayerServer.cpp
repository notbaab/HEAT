#include "PlayerServer.h"
#include "events/PhysicsComponentUpdate.h"
#include "events/PlayerInputEvent.h"
#include "logger/Logger.h"

namespace gameobjects
{

void PlayerServer::Update()
{
    if (moves.empty())
    {
        return;
    }

    // apply any moves we got from the players
    ApplyMoves(physicsComponent, moves);
    lastMoveSeq = moves.back()->moveSeq;
    moves.clear();

    // send an update message. For now it's just the physics component that is
    // being updated.
    auto phyEvt = std::make_shared<PhysicsComponentUpdate>(this->GetWorldId(), lastMoveSeq, this->physicsComponent);
    EventManager::sInstance->QueueEvent(phyEvt);
    TRACE("Sent {}, {}", phyEvt->x, phyEvt->y);
}

void PlayerServer::HandleStateMessage(std::shared_ptr<PhysicsComponentUpdate> stateEvent) {}

} // namespace gameobjects
