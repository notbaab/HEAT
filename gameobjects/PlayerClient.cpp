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
    predictedState->centerLocation.mX = physicsComponent->centerLocation.mX;
    predictedState->centerLocation.mY = physicsComponent->centerLocation.mY;

    TRACE("Snapped predicted state to {}, {}. Applying {} moves", predictedState->centerLocation.mX,
          predictedState->centerLocation.mY, moves.size());

    ApplyMoves(predictedState, moves);

    TRACE("Predicted state ended at to {}, {} ", predictedState->centerLocation.mX, predictedState->centerLocation.mY);
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
          this->physicsComponent->centerLocation.mX, this->physicsComponent->centerLocation.mY, stateEvent->x,
          stateEvent->y);

    this->physicsComponent->centerLocation.mX = stateEvent->x;
    this->physicsComponent->centerLocation.mY = stateEvent->y;
    this->lastMoveSeq = stateEvent->moveSeq;
}

} // namespace gameobjects
