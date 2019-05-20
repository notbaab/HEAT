#pragma once

#include <SDL.h>
#include <memory>
#include <vector>

#include "GraphicsDriver.h"
#include "AnimatedSpriteComponent.h"

template <typename T>
class AnimatedSpriteComponent;

template <typename T>
class RenderManager
{
  public:
    static void StaticInit() { sInstance.reset(new RenderManager()); };
    static inline std::unique_ptr<RenderManager> sInstance;

    void Render()
    {
        SDL_Rect viewport = GraphicsDriver::sInstance->GetLogicalViewport();
        GraphicsDriver::sInstance->Clear();

        // The view transform stores both the scale factor and offset for rendering
        // textures
        mViewTransform.x = viewport.w / 2;
        mViewTransform.y = viewport.h / 2;
        mViewTransform.w = 100;
        mViewTransform.h = 100;

        RenderComponents();

        GraphicsDriver::sInstance->Present();
    };

    // this part that renders the world is really a camera-
    // in a more detailed engine, we'd have a list of cameras, and then render
    // manager would
    // render the cameras in order
    void RenderComponents()
    {
        // Get the logical viewport so we can pass this to the SpriteComponents when
        // it's draw time
        //    SDL_Rect viewport = GraphicsDriver::sInstance->GetLogicalViewport();
        SDL_Renderer* renderer = GraphicsDriver::sInstance->GetRenderer();

        for (auto cIt = mComponents.begin(), end = mComponents.end(); cIt != end; ++cIt)
        {
            (*cIt)->Draw(mViewTransform);
        }

        SDL_SetRenderDrawColor(renderer, 100, 149, 237, SDL_ALPHA_OPAQUE);
    };

    // vert inefficient method of tracking scene graph...
    void AddComponent(AnimatedSpriteComponent<T>* inComponent) { mComponents.push_back(inComponent); };
    void RemoveComponent(AnimatedSpriteComponent<T>* inComponent)
    {
        int index = GetComponentIndex(inComponent);

        if (index != -1)
        {
            int lastIndex = (int)mComponents.size() - 1;
            if (index != lastIndex)
            {
                mComponents[index] = mComponents[lastIndex];
            }
            mComponents.pop_back();
        }
    };

    int GetComponentIndex(AnimatedSpriteComponent<T>* inComponent) const
    {
        for (int i = 0, c = (int)mComponents.size(); i < c; ++i)
        {
            if (mComponents[i] == inComponent)
            {
                return i;
            }
        }

        return -1;
    };

  private:
    // this can't be only place that holds on to component- it has to live
    // inside a GameObject in the world
    std::vector<AnimatedSpriteComponent<T>*> mComponents;

    SDL_Rect mViewTransform;
};
