#include "Player.h"
#include "events/PlayerInputEvent.h"
#include "logger/Logger.h"

namespace gameobjects
{

void Player::AddMove(std::shared_ptr<PlayerInputEvent> evt) { moves.emplace_back(evt); }

void Player::ApplyMoves(std::shared_ptr<PhysicsComponent> component,
                        std::deque<std::shared_ptr<PlayerInputEvent>>& inMoves)
{
    for (auto&& move : inMoves)
    {
        component->centerLocation.x += move->xDelta * 2;
        component->centerLocation.y += move->yDelta * 2;
        TRACE("After move {} at {}, {} ", move->moveSeq, component->centerLocation.x, component->centerLocation.y);
        component->speed.x = move->xDelta;
        component->speed.y = move->yDelta;
    }

    moving = component->IsMoving();
    attacking = false;

    TRACE("At {} ended at {}, {}", lastMoveSeq, component->centerLocation.x, component->centerLocation.y);
}

} // namespace gameobjects
