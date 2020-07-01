#include "WindowManager.h"

std::unique_ptr<WindowManager> WindowManager::sInstance;

bool WindowManager::StaticInit()
{
    SDL_Window* wnd = SDL_CreateWindow("WINDOW!", 100, 100, 1200, 1200, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (wnd == nullptr)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create window.");
        return false;
    }

    sInstance.reset(new WindowManager(wnd));

    return true;
}

WindowManager::WindowManager(SDL_Window* inMainWindow) { mMainWindow = inMainWindow; }

WindowManager::~WindowManager() { SDL_DestroyWindow(mMainWindow); }
