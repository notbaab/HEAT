#pragma once

#include <SDL.h>
#include <functional>
#include <memory>

using KeyCallback = std::function<void(int)>;
using WindowCallback = std::function<void(SDL_WindowEvent& event)>;

class InputManager
{
  public:
    // InputManager();
    // ~InputManager();
    static void StaticInit();

    void RegisterKeyUpListner(KeyCallback callback) { keyUpCallback = callback; }
    void RegisterKeyDownListner(KeyCallback callback) { keyDownCallback = callback; }
    void RegisterWindowListner(WindowCallback callback) { windowEventCallback = callback; }

    void HandleSDLEvent(SDL_Event& event);
    static std::unique_ptr<InputManager> sInstance;

  private:
    // empty event handlers until we override them
    InputManager()
    {
        keyUpCallback = [](int) { return; };
        keyDownCallback = [](int) { return; };
        windowEventCallback = [](SDL_WindowEvent&) { return; };
    };

    KeyCallback keyUpCallback;
    KeyCallback keyDownCallback;

    WindowCallback windowEventCallback;
};
