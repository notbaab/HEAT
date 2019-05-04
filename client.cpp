#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <thread>

#include "graphics/GraphicsDriver.h"
#include "graphics/RenderManager.h"
#include "graphics/SpriteComponent.h"
#include "graphics/WindowManager.h"
#include "math/Vector3.h"

#include "rapidjson/document.h"
#include <rapidjson/istreamwrapper.h>

const char** __argv;
int __argc;

#define TESTSHEET "../images/megaman.png"
#define TESTDATA "../images/megaman-sheet-data.json"

// Loads a sprite sheet into the texture manager and data
class SpriteSheetData
{
  public:
    std::string sheetLoc;
    // vector of a vector of a vector
    SpriteSheetAnimationFrameData frameData;
    SpriteSheetData(std::string sheetLoc, std::string dataLoc) : sheetLoc(sheetLoc)
    {
        std::ifstream ifs(dataLoc);
        rapidjson::IStreamWrapper isw(ifs);
        rapidjson::Document d;
        d.ParseStream(isw);
        if (d.HasParseError())
        {
            std::cout << "error" << std::endl;
            return;
        }

        assert(d.IsArray());

        /// loop over each animation frame
        for (auto& v : d.GetArray())
        {
            auto animation = v.GetObject();
            auto frameName = animation["name"].GetString();
            auto frames = animation["frames"].GetArray();

            // Full animation vector
            AnimationFrameData animationVector;

            std::vector<uint32_t> frameInfo(frames.Size());
            for (auto& frame : frames)
            {
                SingleFrameData info =
                    createSingleFrame(frame["x"].GetInt(), frame["y"].GetInt(),
                                      frame["width"].GetInt(), frame["height"].GetInt());
                animationVector.push_back(info);
            }

            frameData.push_back(animationVector);
        }
    };
};

// Simple drawable game object class
class SimpleGameObject
{
  public:
    SimpleGameObject(SpriteSheetData data)
        : spriteSheetData(data), animationIdx(0), animationStartTime(0)
    {
        centerLocation = Vector3(23, 23, 0);
    };

    Vector3 GetLocation() { return centerLocation; }
    SpriteSheetAnimationFrameData getSpriteSheetFrameLoc() { return spriteSheetData.frameData; }
    std::string getSpriteSheet() { return spriteSheetData.sheetLoc; }

    // Should be protected but for now we don't actually have anything
    // that try's to change the animation
    void changeAnimation(uint32_t idx)
    {
        // TODO: Check bounds of animation data?
        animationIdx = idx;
        framesInCurrentAnimation = spriteSheetData.frameData[animationIdx].size();
        // Should we just peer into sdl from here to get the start time?
        animationStartTime = SDL_GetTicks();
    }

    // look into your spritesheetdata and get the index of the current frame
    // Will be managed by other things the game object is doing
    uint32_t GetCurrentAnimationIdx() const { return animationIdx; }
    uint32_t GetCurrentAnimationFrameIdx(uint32_t sdlTick) const
    {
        // For this, take the start of the animation and subtract from the
        // sdlTick to get when the animation started. Then math to
        // get the next idx

        // Delay should come from the sprite sheet data?
        int delayPerFrame = 80;
        uint32_t totalFrames = spriteSheetData.frameData[animationIdx].size();
        uint32_t adjustedTick = sdlTick - animationStartTime;
        return (adjustedTick / delayPerFrame) % totalFrames;
    }

    // Simple update function called every tick
    void update() {}

    SpriteSheetData spriteSheetData;
    uint32_t animationIdx;
    uint32_t framesInCurrentAnimation;
    uint32_t animationStartTime;
    Vector3 centerLocation;
};

void loadSpriteSheets() {}

// Init all the static things
bool startSDL()
{
    SDL_Init(SDL_INIT_EVERYTHING);

    if (!WindowManager::StaticInit())
    {
        return false;
    }

    if (!GraphicsDriver::StaticInit(WindowManager::sInstance->GetMainWindow()))
    {
        return false;
    }

    RenderManager<SimpleGameObject>::StaticInit();
    return true;
}

void HandleEvent(SDL_Event* event)
{
    //     std::cout << "In event " << event->type << std::endl;
    //     if (event->type == SDL_WINDOWEVENT)
    //     {
    //         switch (event->window.event)
    //         {
    //         case SDL_WINDOWEVENT_SHOWN:
    //             SDL_Log("Window %d shown", event->window.windowID);
    //             break;
    //         case SDL_WINDOWEVENT_HIDDEN:
    //             SDL_Log("Window %d hidden", event->window.windowID);
    //             break;
    //         case SDL_WINDOWEVENT_EXPOSED:
    //             SDL_Log("Window %d exposed", event->window.windowID);
    //             break;
    //         case SDL_WINDOWEVENT_MOVED:
    //             SDL_Log("Window %d moved to %d,%d", event->window.windowID, event->window.data1,
    //                     event->window.data2);
    //             break;
    //         case SDL_WINDOWEVENT_RESIZED:
    //             SDL_Log("Window %d resized to %dx%d", event->window.windowID,
    //             event->window.data1,
    //                     event->window.data2);
    //             break;
    //         case SDL_WINDOWEVENT_SIZE_CHANGED:
    //             SDL_Log("Window %d size changed to %dx%d", event->window.windowID,
    //             event->window.data1,
    //                     event->window.data2);
    //             break;
    //         case SDL_WINDOWEVENT_MINIMIZED:
    //             SDL_Log("Window %d minimized", event->window.windowID);
    //             break;
    //         case SDL_WINDOWEVENT_MAXIMIZED:
    //             SDL_Log("Window %d maximized", event->window.windowID);
    //             break;
    //         case SDL_WINDOWEVENT_RESTORED:
    //             SDL_Log("Window %d restored", event->window.windowID);
    //             break;
    //         case SDL_WINDOWEVENT_ENTER:
    //             SDL_Log("Mouse entered window %d", event->window.windowID);
    //             break;
    //         case SDL_WINDOWEVENT_LEAVE:
    //             SDL_Log("Mouse left window %d", event->window.windowID);
    //             break;
    //         case SDL_WINDOWEVENT_FOCUS_GAINED:
    //             SDL_Log("Window %d gained keyboard focus", event->window.windowID);
    //             break;
    //         case SDL_WINDOWEVENT_FOCUS_LOST:
    //             SDL_Log("Window %d lost keyboard focus", event->window.windowID);
    //             break;
    //         case SDL_WINDOWEVENT_CLOSE:
    //             SDL_Log("Window %d closed", event->window.windowID);
    //             break;
    // #if SDL_VERSION_ATLEAST(2, 0, 5)
    //         case SDL_WINDOWEVENT_TAKE_FOCUS:
    //             SDL_Log("Window %d is offered a focus", event->window.windowID);
    //             break;
    //         case SDL_WINDOWEVENT_HIT_TEST:
    //             SDL_Log("Window %d has a special hit test", event->window.windowID);
    //             break;
    // #endif
    //         default:
    //             SDL_Log("Window %d got unknown event %d", event->window.windowID,
    //             event->window.event); break;
    //         }
    //     }
}
bool Loop()
{
    auto spriteData = SpriteSheetData(TESTSHEET, TESTDATA);
    auto simpleGameObject = SimpleGameObject(spriteData);
    auto spriteComponent = SpriteComponent(&simpleGameObject);
    RenderManager<SimpleGameObject>::sInstance->AddComponent(&spriteComponent);

    while (true)
    {

        // Main message loop
        SDL_Event event;
        memset(&event, 0, sizeof(SDL_Event));
        if (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                SDL_Quit();
                return false;
            }
            else
            {
                simpleGameObject.changeAnimation(rand() % 3);
                HandleEvent(&event);
            }
        }

        RenderManager<SimpleGameObject>::sInstance->Render();
    }
}

int main(int argc, const char* argv[])
{
    __argc = argc;
    __argv = argv;

    if (!startSDL())
    {
        std::cout << "NOPE" << std::endl;
        SDL_Quit();
    };

    Loop();
}
