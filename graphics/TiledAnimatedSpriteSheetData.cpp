#include "TiledAnimatedSpriteSheetData.h"

AnimationOrientation AnimationOrientationFromString(std::string orientationStr)
{
    if (orientationStr.compare("up") == 0)
    {
        return AnimationOrientation::UP;
    }
    else if (orientationStr.compare("down"))
    {
        return AnimationOrientation::DOWN;
    }
    else if (orientationStr.compare("left"))
    {
        return AnimationOrientation::LEFT;
    }
    else if (orientationStr.compare("right"))
    {
        return AnimationOrientation::RIGHT;
    }

    return AnimationOrientation::NONE;
}