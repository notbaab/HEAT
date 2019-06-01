#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>

#include "controls/InputManager.h"
#include "graphics/AnimatedSpriteComponent.h"
#include "graphics/GraphicsDriver.h"
#include "graphics/RenderManager.h"
#include "graphics/SpriteSheetData.h"
#include "graphics/StaticSpriteComponent.h"
#include "graphics/WindowManager.h"
#include "math/Vector3.h"

const char** __argv;
int __argc;

#define TESTANIMATEDSHEET "../images/megaman.png"
#define TESTANIMATEDDATA "../images/megaman-sheet-data.json"

#define TESTSTATICSHEET "../images/shipsMiscellaneous_sheet.png"
#define TESTSTATICSHEETDATA "../images/ship-sheet.json"

#define TESTSHIP "ship (24).png"

// Simple drawable game object class
class SimpleGameObject
{
  public:
    SimpleGameObject() : rotation(0) { centerLocation = Vector3(23, 23, 0); };

    Vector3 GetLocation() { return centerLocation; }
    // std::string getSpriteSheet() { return spriteSheetData.sheetLoc; }
    void SetRotation(uint16_t degress) { rotation = degress; }
    uint16_t GetRotation() { return rotation; }

    // Simple update function called every tick
    void update() {}

    // Animated sprite sheet data and regular sprite sheet data
    uint16_t rotation;
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

    RenderManager::StaticInit();
    return true;
}

void KeyPressed(int keyCode) { std::cout << "Key pressed " << keyCode << std::endl; }
void KeyReleased(int keyCode) { std::cout << "Key pressed " << keyCode << std::endl; }

bool Loop()
{
    auto pirateSheet = SpriteSheetData(TESTSTATICSHEET, TESTSTATICSHEETDATA);
    auto megaManSheet = SpriteSheetData(TESTANIMATEDSHEET, TESTANIMATEDDATA);

    auto pirateShip = SimpleGameObject();
    auto megaMan = SimpleGameObject();

    auto pirateShipRect = pirateSheet.staticTextureMap[TESTSHIP];

    auto pirateComponent = StaticSpriteComponent(&pirateShip, TESTSTATICSHEET, pirateShipRect);
    auto megaManComponent =
        AnimatedSpriteComponent(&megaMan, TESTANIMATEDSHEET, megaManSheet.animations);

    megaManComponent.ChangeAnimation("first");
    RenderManager::sInstance->AddComponent(&pirateComponent);
    RenderManager::sInstance->AddComponent(&megaManComponent);

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
                // simpleGameObject.changeAnimation(rand() % 3);
                pirateShip.SetRotation(rand() % 360);

                InputManager::sInstance->HandleSDLEvent(event);
            }
        }

        RenderManager::sInstance->Render();
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
