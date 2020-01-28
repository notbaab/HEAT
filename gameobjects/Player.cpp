#include "Player.h"
#include "events/PlayerInputEvent.h"
#include "logger/Logger.h"

namespace gameobjects
{

void Player::HandleInput(std::shared_ptr<PlayerInputEvent> evt)
{
    DEBUG("Handing input {} {}", evt->xDelta, physicsComponent->centerLocation.mX);
    // take the input and apply it to the next state the player will be
    physicsComponent->centerLocation.mX += evt->xDelta;
    physicsComponent->centerLocation.mY += evt->yDelta;
}

void Player::Update()
{
    // queue up a move to be processed in the players update event
}

} // namespace gameobjects
