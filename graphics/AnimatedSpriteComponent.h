#pragma once
#include <SDL_image.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "AssetManager.h"
#include "DrawableComponent.h"
#include "GraphicsDriver.h"
#include "TiledAnimatedSpriteSheetData.h"
#include "logger/Logger.h"
#include "math/Vector3.h"

template <typename T>
class AnimatedSpriteComponent : public DrawableComponent
{
  public:
    AnimatedSpriteComponent(T* inGameObject,
                            std::shared_ptr<TiledAnimatedSpriteSheetData> sheetData,
                            bool justOutline)

    {
        mGameObject = inGameObject;
        animationFrameData = &sheetData->animations;
        this->justOutline = justOutline;

        // It is worth it to explore some sort of texture manager to cache textures.
        // Doing it this way means we load the texture for every sprite component.
        mTexture = AssetManager::sInstance->GetTexture(sheetData->name);
        // mTexture = IMG_LoadTexture(GraphicsDriver::sInstance->GetRenderer(),
        // "images/megaman.png"); IMG_LoadTexture(GraphicsDriver::sInstance->GetRenderer(),
        // sheetData->sheetLoc.c_str());
    }

    AnimatedSpriteComponent(T* inGameObject,
                            std::shared_ptr<TiledAnimatedSpriteSheetData> sheetData)
        : AnimatedSpriteComponent(inGameObject, sheetData, false)
    {
    }
    ~AnimatedSpriteComponent() { SDL_DestroyTexture(mTexture); }

    SDL_Rect getCurrentAnimationFrame()
    {
        // Group these together in a single call?
        uint32_t currentTick = SDL_GetTicks();
        uint32_t drawTime = currentTick - currentFrameStart;
        uint32_t frameDuration = (*currentAnimation)[frameIndex].duration;
        if (drawTime >= frameDuration)
        {
            frameIndex++;
            frameIndex %= framesInAnimation;
            currentFrameStart = currentTick;
        }

        // PIMP Wtf is this shit, this is hideous
        SDL_Rect textureRect;
        textureRect.x = (*currentAnimation)[frameIndex].drawRect.x;
        textureRect.y = (*currentAnimation)[frameIndex].drawRect.y;
        textureRect.w = (*currentAnimation)[frameIndex].drawRect.width;
        textureRect.h = (*currentAnimation)[frameIndex].drawRect.height;

        return textureRect;
    }

    // TODO: Should it be a string still?
    void ChangeAnimation(std::string animation)
    {
        currentAnimationData = &(*animationFrameData)[animation];
        currentAnimation = &currentAnimationData->animations;
        framesInAnimation = currentAnimationData->NumAnimations();
        currentFrameStart = SDL_GetTicks();
        frameIndex = 0;
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
        Vector3 objLocation = mGameObject->centerLocation;
        TRACE("Object at {} {}", objLocation.mX, objLocation.mY);
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

        DrawSpriteOutline();
        if (justOutline)
        {
            return;
        }

        SDL_RenderCopy(renderer, mTexture, &frame, &currentFrameRect);
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
    // SpriteSheetAnimationFrameData* animationFrameData;sheetName

    std::unordered_map<std::string, SpriteAnimationData>* animationFrameData;
    // Book keeping for the current animation that is playing
    SpriteAnimationData* currentAnimationData;

    std::vector<SpriteAnimationFrameData>* currentAnimation;
    uint32_t frameIndex;
    uint32_t currentFrameStart;
    uint32_t framesInAnimation;

    SDL_Rect currentFrameRect;
    bool justOutline;

    // I don't want circular reference...
    T* mGameObject;
};
