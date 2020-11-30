#pragma once

#include "DrawableComponent.h"
#include "GraphicsDriver.h"
#include "math/Vector3.h"
#include <SDL_image.h>
#include <memory>
#include <string>
#include <vector>

// static sprite component is a component that use a static texture map to determine what to draw.
// This could be used for scenery or components without animation. Overall I'm not really sure
// if this is a great approach. I need some more graphics people advice
template <typename T>
class StaticSpriteComponent : public DrawableComponent
{
  public:
    StaticSpriteComponent(T* inGameObject, std::string spriteSheetLoc, std::vector<uint32_t> drawRect)
    {
        mGameObject = inGameObject;

        mTextureRect.x = drawRect[0];
        mTextureRect.y = drawRect[1];
        mTextureRect.w = drawRect[2];
        mTextureRect.h = drawRect[3];
        mHalfTextureWidth = mTextureRect.w / 2;
        mHalfTextureHeight = mTextureRect.h / 2;

        // It is worth it to explore some sort of texture manager to cache textures.
        // Doing it this way means we load the texture for every sprite component.
        mTexture = IMG_LoadTexture(GraphicsDriver::sInstance->GetRenderer(), spriteSheetLoc.c_str());
    }

    ~StaticSpriteComponent() {}

    void Draw(const SDL_Rect& inViewTransform)
    {
        // I kinda think we should just hold a reference to the renderer.
        // I can't imagine a scenario where we change it out once we start
        SDL_Renderer* renderer = GraphicsDriver::sInstance->GetRenderer();

        // What size should we render the texture? We probably will need to
        // flesh out the game object to get a better sense of how we should
        // do that.
        Vector3 objLocation = mGameObject->GetLocation();
        uint16_t rotation = mGameObject->GetRotation();

        // top right coordinates should be offset by the size of the frame.
        // This produces kinda a weird effect with odd sized frames
        mWorldRect.x = objLocation.x - mHalfTextureWidth;
        mWorldRect.y = objLocation.y - mHalfTextureHeight;
        mWorldRect.w = mTextureRect.w;
        mWorldRect.h = mTextureRect.h;

        // SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        // SDL_RenderCopy(renderer, mTexture, &mTextureRect, &mWorldRect);
        SDL_RenderCopyEx(renderer, mTexture, &mTextureRect, &mWorldRect, rotation, NULL, SDL_FLIP_NONE);
        DrawSpriteOutline();
    }

    // Draws a square around the sprite dimensions.
    void DrawSpriteOutline()
    {
        SDL_Renderer* renderer = GraphicsDriver::sInstance->GetRenderer();
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &mWorldRect);
    }

  private:
    SDL_Rect mTextureRect;
    SDL_Rect mWorldRect;
    uint32_t mHalfTextureWidth, mHalfTextureHeight;

    SDL_Texture* mTexture;

    // hmmm. This should probably be a weak ptr if we want to be all c++11 proper
    T* mGameObject;
};
