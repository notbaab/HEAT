#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>

// Yeah this is too many includes
#include "IO/InputMemoryBitStream.h"
#include "IO/OutputMemoryBitStream.h"
#include "controls/InputManager.h"
#include "engine/Engine.h"
#include "events/Event.h"
#include "events/EventManager.h"
#include "gameobjects/Player.h"
#include "gameobjects/Registry.h"
#include "gameobjects/SimpleGameObject.h"
#include "gameobjects/World.h"
#include "graphics/AnimatedSpriteComponent.h"
#include "graphics/GraphicsDriver.h"
#include "graphics/RenderManager.h"
#include "graphics/SpriteSheetData.h"
#include "graphics/StaticSpriteComponent.h"
#include "graphics/WindowManager.h"
#include "holistic/SetupFuncs.h"
#include "logger/Logger.h"
#include "managers/NetworkManagerClient.h"
#include "managers/PacketManager.h"
#include "math/Vector3.h"
#include "messages/ClientConnectionChallengeMessage.h"
#include "messages/ClientLoginMessage.h"
#include "messages/ClientLoginResponse.h"
#include "messages/ClientWelcomeMessage.h"
#include "messages/PlayerMessage.h"
#include "networking/SocketAddressFactory.h"
#include "networking/SocketManager.h"
#include "packets/AuthenticatedPacket.h"
#include "packets/Message.h"
#include "packets/MessageSerializer.h"
#include "packets/Packet.h"
#include "packets/PacketSerializer.h"
#include "packets/ReliableOrderedPacket.h"
#include "packets/UnauthenticatedPacket.h"

const char** __argv;
int __argc;

#define TESTANIMATEDSHEET "images/megaman.png"
#define TESTANIMATEDDATA "images/megaman-sheet-data.json"

#define TESTSTATICSHEET "images/shipsMiscellaneous_sheet.png"
#define TESTSTATICSHEETDATA "images/ship-sheet.json"

#define TESTSHIP "ship (24).png"

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

//
bool DoFrame()
{
    NetworkManagerClient::sInstance->ProcessMessages();
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
            // pirateShip.SetRotation(rand() % 360);

            InputManager::sInstance->HandleSDLEvent(event);
            // Read some packets and do stuffs
        }
    }

    // TODO: Add timing
    EventManager::sInstance->FireEvents(10);

    RenderManager::sInstance->Render();

    NetworkManagerClient::sInstance->SendOutgoingPackets();
    NetworkManagerClient::sInstance->packetManager.StepTime(0.03);
    return true;
}

void AddGameObjectToWorld(GameObjectPtr ptr) {}

void SetupNetworking(std::string serverDestination)
{
    // Make a test packet and see if it makes it to the other side
    auto messageSerializer = std::make_shared<MessageSerializer>();
    AddMessageCtor(messageSerializer, PlayerMessage);
    AddMessageCtor(messageSerializer, ClientConnectionChallengeMessage);
    AddMessageCtor(messageSerializer, ClientLoginMessage);
    AddMessageCtor(messageSerializer, ClientLoginResponse);

    // Event constructors. Events are also messages
    auto packetSerializer = std::make_shared<PacketSerializer>(messageSerializer);
    AddPacketCtor(packetSerializer, ReliableOrderedPacket);
    AddPacketCtor(packetSerializer, UnauthenticatedPacket);
    AddPacketCtor(packetSerializer, AuthenticatedPacket);

    NetworkManagerClient::StaticInit(serverDestination, packetSerializer, "Joe Mamma");
}
void SetupEventListeners()
{
    EventManager::StaticInit();
}
void initStuffs()
{
    logger::InitLog(logger::level::TRACE, "Main Logger");
    if (!startSDL())
    {
        std::cout << "NOPE" << std::endl;
        SDL_Quit();
    };

    DEBUG("Starting Client")
    holistic::SetupWorld();

    std::string destination = "127.0.0.1:4500";

    SetupNetworking(destination);
    SetupEventListeners();

    NetworkManagerClient::sInstance->StartServerHandshake();
    NetworkManagerClient::sInstance->SendOutgoingPackets();

    // auto pirateSheet = SpriteSheetData(TESTSTATICSHEET, TESTSTATICSHEETDATA);
    // auto megaManSheet = SpriteSheetData(TESTANIMATEDSHEET, TESTANIMATEDDATA);

    // auto pirateShip = SimpleGameObject();
    // auto megaMan = SimpleGameObject();

    // auto pirateShipRect = pirateSheet.staticTextureMap[TESTSHIP];

    // auto pirateComponent = StaticSpriteComponent(&pirateShip, TESTSTATICSHEET, pirateShipRect);
    // auto megaManComponent =
    //     AnimatedSpriteComponent(&megaMan, TESTANIMATEDSHEET, megaManSheet.animations);

    // megaManComponent.ChangeAnimation("first");
    // RenderManager::sInstance->AddComponent(&pirateComponent);
    // RenderManager::sInstance->AddComponent(&megaManComponent);

    InputManager::StaticInit();
    InputManager::sInstance->RegisterKeyDownListner(KeyPressed);
    InputManager::sInstance->RegisterKeyUpListner(KeyReleased);
}

int main(int argc, const char* argv[])
{
    __argc = argc;
    __argv = argv;

    Engine engine = Engine(initStuffs, DoFrame);
    engine.Run();
}
