#include "Player.h"
#include "events/PlayerInputEvent.h"
#include "logger/Logger.h"

namespace gameobjects
{

// TODO: This doesn't actually ensure moves are in ascending move seq order
void Player::AddMove(std::shared_ptr<PlayerInputEvent> evt) { moves.emplace_back(evt); }

uint32_t Player::ApplyMoves(std::shared_ptr<PhysicsComponent> component,
                            std::deque<std::shared_ptr<PlayerInputEvent>>& inMoves)
{
    uint32_t lastMoveSeqApplied = 0;
    for (auto&& move : inMoves)
    {
        component->centerLocation.x += move->xDelta * 2;
        component->centerLocation.y += move->yDelta * 2;
        TRACE("After move {} at {}, {} ", move->moveSeq, component->centerLocation.x, component->centerLocation.y);
        component->speed.x = move->xDelta;
        component->speed.y = move->yDelta;
        lastMoveSeqApplied = move->moveSeq;
    }

    moving = component->IsMoving();
    attacking = false;

    TRACE("At {} ended at {}, {}", lastMoveSeq, component->centerLocation.x, component->centerLocation.y);
    return lastMoveSeqApplied;
}

} // namespace gameobjects
