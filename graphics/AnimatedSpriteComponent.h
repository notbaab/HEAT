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
#include "gameobjects/PhysicsComponent.h"
#include "logger/Logger.h"
#include "math/Vector3.h"

template <typename T>
class AnimatedSpriteComponent : public DrawableComponent
{
  public:
    AnimatedSpriteComponent(T* inGameObject, std::shared_ptr<TiledAnimatedSpriteSheetData> sheetData, bool justOutline)
        : sheetData(sheetData)
    {
        mGameObject = inGameObject;
        animationFrameData = &sheetData->animations;
        // PIMP: This is so we can have a server ghost of just the outline, probably a much better
        // way
        this->justOutline = justOutline;

        mTexture = AssetManager::sInstance->GetTexture(sheetData->name);

        // this->ChangeAnimation(sheetData->firstAnimation);
        this->ChangeAnimation(MovementType::IDLE, MovementOrientation::NONE);
    }

    AnimatedSpriteComponent(T* inGameObject, std::shared_ptr<TiledAnimatedSpriteSheetData> sheetData)
        : AnimatedSpriteComponent(inGameObject, sheetData, false)
    {
    }
    ~AnimatedSpriteComponent() {}

    SDL_Rect getCurrentAnimationFrame()
    {
        MovementType newMovementType = mGameObject->GetCurrentMovementType();
        MovementOrientation newOrientation = mGameObject->GetCurrentOrientation();

        if (newMovementType != currentType || newOrientation != currentOrientation)
        {
            ChangeAnimation(newMovementType, newOrientation);
        }

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

    void ChangeAnimation(MovementType type, MovementOrientation orientation)
    {
        // Set it even if we don't have an animation for this type
        currentType = type;
        if (type == MovementType::IDLE)
        {
            // TODO: This is wrong, we should have idle animations for all orientations.
            currentOrientation = MovementOrientation::NONE;
        }
        else
        {
            currentOrientation = orientation;
        }

        auto animation = sheetData->GetAnimation(currentType, currentOrientation);
        if (animation == nullptr)
        {
            ERROR("No animation found for {} {}", currentType, currentOrientation);
            return;
        }

        TRACE("Changing animation to {} {}", type, orientation);
        assert(animation);

        currentAnimationData = animation.get();
        currentAnimation = &currentAnimationData->animations;
        framesInAnimation = currentAnimationData->NumAnimations();
        currentFrameStart = SDL_GetTicks();
        frameIndex = 0;
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
        Vector3 objLocation = mGameObject->GetLocation();
        TRACE("Object at {} {}", objLocation.x, objLocation.y);
        uint32_t halfWidth = frame.w / 2;
        uint32_t halfHeight = frame.h / 2;

        // top right coordinates should be offset by the size of the frame.
        // This produces kinda a weird effect with odd sized frames
        currentFrameRect.x = objLocation.x - halfWidth;
        currentFrameRect.y = objLocation.y - halfHeight;
        currentFrameRect.w = frame.w;
        currentFrameRect.h = frame.h;

        // SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        DrawSpriteOutline();
        if (justOutline)
        {
            return;
        }

        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (currentAnimationData->flipped)
        {
            // For now flip horizontally
            flip = SDL_FLIP_HORIZONTAL;
        }

        SDL_RenderCopyEx(renderer, mTexture, &frame, &currentFrameRect, 0, NULL, flip);
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
        mColor.x = inColor->x;
        mColor.y = inColor->y;
        mColor.z = inColor->z;
    }

  private:
    Vector3 mColor;

    SDL_Texture* mTexture;
    // Animation Frame Data
    std::shared_ptr<TiledAnimatedSpriteSheetData> sheetData;

    std::unordered_map<std::string, std::shared_ptr<SpriteAnimationData>>* animationFrameData;
    // Book keeping for the current animation that is playing
    SpriteAnimationData* currentAnimationData;

    std::vector<SpriteAnimationFrameData>* currentAnimation;
    uint32_t frameIndex;
    uint32_t currentFrameStart;
    uint32_t framesInAnimation;

    MovementType currentType;
    MovementOrientation currentOrientation;

    SDL_Rect currentFrameRect;
    bool justOutline;

    // I don't want circular reference...
    T* mGameObject;
};
