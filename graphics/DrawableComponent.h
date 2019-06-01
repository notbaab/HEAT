#pragma once
#include <SDL.h>

class DrawableComponent
{
  public:
    virtual void Draw(const SDL_Rect& inViewTransform) = 0;
};
