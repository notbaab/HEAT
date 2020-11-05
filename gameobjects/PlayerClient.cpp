#include "PlayerClient.h"
#include "events/PhysicsComponentUpdate.h"
#include "events/PlayerInputEvent.h"
#include "logger/Logger.h"

namespace gameobjects
{

void PlayerClient::RemoveOldMoves(std::deque<std::shared_ptr<PlayerInputEvent>>& inMoves)
{
    auto& move = inMoves.front();
    TRACE("seq {} ", move->moveSeq);
    while (move->moveSeq <= lastMoveSeq)
    {
        TRACE("Removing move {} since lower than {}", move->moveSeq, lastMoveSeq);
        inMoves.pop_front();
        if (inMoves.empty())
        {
            break;
        }
        move = inMoves.front();
    }
}

void PlayerClient::PredictState()
{
    predictedState->centerLocation.x = physicsComponent->centerLocation.x;
    predictedState->centerLocation.y = physicsComponent->centerLocation.y;

    TRACE("Snapped predicted state to {}, {}. Applying {} moves", predictedState->centerLocation.x,
          predictedState->centerLocation.y, moves.size());

    ApplyMoves(predictedState, moves);

    TRACE("Predicted state ended at to {}, {} ", predictedState->centerLocation.x, predictedState->centerLocation.y);
}

MovementOrientation PlayerClient::GetCurrentOrientation() { return physicsComponent->GetCurrentOrientation(); }

MovementType PlayerClient::GetCurrentMovementType()
{
    if (attacking)
    {
        return MovementType::ATTACK;
    }
    if (moving)
    {
        return MovementType::WALK;
    }
    return MovementType::IDLE;
}

void PlayerClient::Update()
{
    // remove any moves that we need to
    if (moves.empty())
    {
        // nothing to do
        return;
    }

    RemoveOldMoves(moves);
    PredictState();
}

void PlayerClient::HandleStateMessage(std::shared_ptr<PhysicsComponentUpdate> stateEvent)
{
    // snap movement to the state event.
    TRACE("Handling state, moving to moveSeq {} from {},{} to {},{}", stateEvent->moveSeq,
          this->physicsComponent->centerLocation.x, this->physicsComponent->centerLocation.y, stateEvent->x,
          stateEvent->y);

    this->physicsComponent->centerLocation.x = stateEvent->x;
    this->physicsComponent->centerLocation.y = stateEvent->y;
    this->lastMoveSeq = stateEvent->moveSeq;

    this->physicsComponent->speed.x = stateEvent->dX;
    this->physicsComponent->speed.y = stateEvent->dY;
}

} // namespace gameobjects
