#include <string>

#include "Vector3.h"

const Vector3 Vector3::Zero(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::UnitX(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::UnitY(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::UnitZ(0.0f, 0.0f, 1.0f);

MovementOrientation MovementOrientationFromString(std::string orientationStr)
{
    if (orientationStr.compare("up") == 0)
    {
        return MovementOrientation::UP;
    }
    else if (orientationStr.compare("down") == 0)
    {
        return MovementOrientation::DOWN;
    }
    else if (orientationStr.compare("left") == 0)
    {
        return MovementOrientation::LEFT;
    }
    else if (orientationStr.compare("right") == 0)
    {
        return MovementOrientation::RIGHT;
    }

    return MovementOrientation::NONE;
}

MovementType MovementTypeFromString(std::string animType)
{
    if (animType.compare("idle") == 0)
    {
        return MovementType::IDLE;
    }
    else if (animType.compare("walk") == 0)
    {
        return MovementType::WALK;
    }
    else if (animType.compare("attack") == 0)
    {
        return MovementType::ATTACK;
    }

    return MovementType::NONE;
}

MovementOrientation OrientationFromVector(Vector3 movement)
{

    if (movement.x == 0 && movement.y == 0)
    {
        return MovementOrientation::NONE;
    }
    else if (movement.x > 0 && movement.y == 0)
    {
        return MovementOrientation::RIGHT;
    }
    else if (movement.x < 0 && movement.y == 0)
    {
        return MovementOrientation::LEFT;
    }
    else if (movement.x == 0 && movement.y > 0)
    {
        return MovementOrientation::DOWN;
    }
    else if (movement.x == 0 && movement.y < 0)
    {
        return MovementOrientation::UP;
    }
    else if (movement.x > 0 && movement.y > 0)
    {
        return MovementOrientation::DOWN_RIGHT;
    }
    else if (movement.x > 0 && movement.y < 0)
    {
        return MovementOrientation::UP_RIGHT;
    }
    else if (movement.x < 0 && movement.y > 0)
    {
        return MovementOrientation::DOWN_LEFT;
    }
    else if (movement.x < 0 && movement.y < 0)
    {
        return MovementOrientation::UP_LEFT;
    }
    return MovementOrientation::NONE;
}

MovementOrientation FlippedOrientation(MovementOrientation orientation)
{

    if (orientation == MovementOrientation::NONE)
    {
        return MovementOrientation::NONE;
    }
    else if (orientation == MovementOrientation::RIGHT)
    {
        return MovementOrientation::LEFT;
    }
    else if (orientation == MovementOrientation::LEFT)
    {
        return MovementOrientation::RIGHT;
    }
    else if (orientation == MovementOrientation::UP)
    {
        return MovementOrientation::DOWN;
    }
    else if (orientation == MovementOrientation::DOWN)
    {
        return MovementOrientation::UP;
    }

    return MovementOrientation::NONE;
}
