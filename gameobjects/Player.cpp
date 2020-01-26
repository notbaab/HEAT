#include "Player.h"
#include "events/PlayerInputEvent.h"

namespace gameobjects
{

void Player::HandleInput(std::shared_ptr<PlayerInputEvent> evt)
{
    // queue up a move to be processed in the players update event
}

void Player::Update()
{
    // queue up a move to be processed in the players update event
}

} // namespace gameobjects
