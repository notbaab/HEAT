#include <iostream>

#include "InputManager.h"

std::unique_ptr<InputManager> InputManager::sInstance;
void InputManager::StaticInit() { sInstance.reset(new InputManager()); }

void InputManager::HandleSDLEvent(SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_WINDOWEVENT:
        SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Window Event Type %d", event.window.type);
        windowEventCallback(event.window);
        break;
    case SDL_KEYDOWN:
        if (event.key.repeat)
        {
            // repeats cause a delay, don't use them
            return;
        }
        SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "KeyPress %d->%d", event.window.windowID, event.key.keysym.sym);
        keyDownCallback(event.key.keysym.sym);
        break;
    case SDL_KEYUP:
        SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "KeyUp %d->%d", event.window.windowID, event.key.keysym.sym);
        keyUpCallback(event.key.keysym.sym);
        break;
    default:
        SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "unhandled event %d", event.window.windowID);
        break;
    }
}
