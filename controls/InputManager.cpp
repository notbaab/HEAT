#include "InputManager.h"
#include <iostream>

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
        SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "KeyPress %d->%d", event.window.windowID,
                       event.key.keysym.sym);
        keyDownCallback(event.key.keysym.sym);
        break;
    case SDL_KEYUP:
        SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "KeyPress %d->%d", event.window.windowID,
                       event.key.keysym.sym);
        keyUpCallback(event.key.keysym.sym);
        break;
    default:
        SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "unhandled event %d", event.window.windowID);
        break;
    }
}