#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <thread>

#include "controls/InputManager.h"
#include "graphics/GraphicsDriver.h"
#include "graphics/RenderManager.h"
#include "graphics/AnimatedSpriteComponent.h"
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

void KeyPressed(int keyCode) { std::cout << "Key pressed " << keyCode << std::endl; }
void KeyReleased(int keyCode) { std::cout << "Key pressed " << keyCode << std::endl; }

bool Loop()
{
    auto spriteData = SpriteSheetData(TESTSHEET, TESTDATA);
    auto simpleGameObject = SimpleGameObject(spriteData);
    auto spriteComponent = AnimatedSpriteComponent(&simpleGameObject);
    RenderManager<SimpleGameObject>::sInstance->AddComponent(&spriteComponent);

    InputManager::StaticInit();
    InputManager::sInstance->RegisterKeyDownListner(KeyPressed);
    InputManager::sInstance->RegisterKeyUpListner(KeyReleased);

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
                InputManager::sInstance->HandleSDLEvent(event);
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
