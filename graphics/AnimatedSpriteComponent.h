#pragma once

#include "GraphicsDriver.h"
#include "math/Vector3.h"
#include <SDL_image.h>
#include <memory>
#include <string>
#include <vector>

// Should be a four element vector in the order of x, y, width, height
typedef std::vector<uint32_t> SingleFrameData;

// A vector single frame data designed to be played as an animation
typedef std::vector<SingleFrameData> AnimationFrameData;

// Holds all animations read from a sprite sheet with the animation data
// indexed
typedef std::vector<AnimationFrameData> SpriteSheetAnimationFrameData;

SingleFrameData createSingleFrame(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    SingleFrameData frameInfo = {x, y, width, height};
    return frameInfo;
}

template <typename T>
class AnimatedSpriteComponent
{
  public:
    AnimatedSpriteComponent(T* inGameObject)
    {
        mGameObject = inGameObject;
        mFrames = inGameObject->getSpriteSheetFrameLoc();
        std::string spriteSheet = inGameObject->getSpriteSheet();

        // It is worth it to explore some sort of texture manager to cache textures.
        // Doing it this way means we load the texture for every sprite component.
        mTexture = IMG_LoadTexture(GraphicsDriver::sInstance->GetRenderer(), spriteSheet.c_str());
    }

    ~AnimatedSpriteComponent() { SDL_DestroyTexture(mTexture); }

    SDL_Rect getCurrentAnimationFrame()
    {
        // Group these together in a single call?
        uint32_t animationIdx = mGameObject->GetCurrentAnimationIdx();
        uint32_t frameIdx = mGameObject->GetCurrentAnimationFrameIdx(SDL_GetTicks());
        auto frame = mFrames[animationIdx][frameIdx];

        SDL_Rect textureRect;
        textureRect.x = frame[0];
        textureRect.y = frame[1];
        textureRect.w = frame[2];
        textureRect.h = frame[3];
        return textureRect;
    }

    void Draw(const SDL_Rect& inViewTransform)
    {
        // I kinda think we should just hold a reference to the renderer.
        // I can't imagine a scenario where we change it out once we start
        SDL_Renderer* renderer = GraphicsDriver::sInstance->GetRenderer();
        SDL_Rect frame = getCurrentAnimationFrame();

        // What size should we render the texture? We probably will need to
        // flesh out the game object to get a better sense of how we should
        // do that.
        Vector3 objLocation = mGameObject->GetLocation();
        uint32_t halfWidth = frame.w / 2;
        uint32_t halfHeight = frame.h / 2;

        // top right coordinates should be offset by the size of the frame.
        // This produces kinda a weird effect with odd sized frames
        currentFrameRect.x = objLocation.mX - halfWidth;
        currentFrameRect.y = objLocation.mY - halfHeight;
        currentFrameRect.w = frame.w;
        currentFrameRect.h = frame.h;

        // SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        SDL_RenderCopy(renderer, mTexture, &frame, &currentFrameRect);
        DrawSpriteOutline();
    }

    // Draws a square around the sprite dimensions.
    void DrawSpriteOutline()
    {
        SDL_Renderer* renderer = GraphicsDriver::sInstance->GetRenderer();
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &currentFrameRect);
    }

    void SetColor(Vector3* inColor)
    {
        mColor.mX = inColor->mX;
        mColor.mY = inColor->mY;
        mColor.mZ = inColor->mZ;
    }

  private:
    Vector3 mColor;

    SDL_Texture* mTexture;
    // Animation Frame Data
    SpriteSheetAnimationFrameData mFrames;

    SDL_Rect currentFrameRect;

    // I don't want circular reference...
    T* mGameObject;
};
