#pragma once
#include <SDL_image.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "DrawableComponent.h"
#include "GraphicsDriver.h"
#include "SpriteSheetData.h"
#include "math/Vector3.h"

template <typename T>
class AnimatedSpriteComponent : public DrawableComponent
{
  public:
    AnimatedSpriteComponent(T* inGameObject, std::string spriteSheetLoc,
                            SpriteSheetAnimationFrameData frameData)
    {
        mGameObject = inGameObject;
        mFrames = frameData;

        // It is worth it to explore some sort of texture manager to cache textures.
        // Doing it this way means we load the texture for every sprite component.
        mTexture =
            IMG_LoadTexture(GraphicsDriver::sInstance->GetRenderer(), spriteSheetLoc.c_str());
    }

    ~AnimatedSpriteComponent() { SDL_DestroyTexture(mTexture); }

    SDL_Rect getCurrentAnimationFrame()
    {
        // Group these together in a single call?
        int delayPerFrame = 80;
        uint32_t adjustedTick = SDL_GetTicks() - animationStartTime;
        uint32_t frameIdx = (adjustedTick / delayPerFrame) % framesInAnimation;

        SDL_Rect textureRect;
        textureRect.x = currentAnimation[frameIdx][0];
        textureRect.y = currentAnimation[frameIdx][1];
        textureRect.w = currentAnimation[frameIdx][2];
        textureRect.h = currentAnimation[frameIdx][3];

        return textureRect;
    }

    // TODO: Should it be a string still?
    void ChangeAnimation(std::string animation)
    {
        currentAnimation = mFrames[animation];
        framesInAnimation = mFrames[animation].size();
        animationStartTime = SDL_GetTicks();
        animationIdx = 0;
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
    // Book keeping for the current animation that is playing
    AnimationFrameData currentAnimation;
    uint32_t animationIdx;
    uint32_t animationStartTime;
    uint32_t framesInAnimation;

    SDL_Rect currentFrameRect;

    // I don't want circular reference...
    T* mGameObject;
};
