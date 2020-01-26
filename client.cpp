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
#include "events/CreatePlayerOwnedObject.h"
#include "events/Event.h"
#include "events/EventManager.h"
#include "events/PhysicsComponentUpdate.h"
#include "events/PlayerInputEvent.h"
#include "gameobjects/PlayerClient.h"
#include "gameobjects/Registry.h"
#include "gameobjects/SetupGameObjects.h"
#include "gameobjects/SimpleGameObject.h"
#include "gameobjects/WorldClient.h"
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

class InputButtonState
{
  public:
    InputButtonState() : up(false), down(false), left(false), right(false)
    {
        // initialize our keyboard mapping, I'm cheating right now since I
        // know the sdl symbols
        // w
        keyMap[119] = &up;
        // s
        keyMap[115] = &down;
        // a
        keyMap[97] = &left;
        // d
        keyMap[100] = &right;
    }

    bool hasInput() { return GetHorizontalDirection() != 0 || GetVerticalDirection() != 0; }
    // if we have input, make a PlayerInputState and queue it into the events
    void QueueInputEvents()
    {
        if (hasInput())
        {
            auto inputState = std::make_shared<PlayerInputEvent>(GetHorizontalDirection(),
                                                                 GetVerticalDirection(), moveSeq);
            moveSeq++;
            EventManager::sInstance->QueueEvent(inputState);
        }
    }
    int8_t GetHorizontalDirection() const { return left - right; };
    int8_t GetVerticalDirection() const { return up - down; };

    void KeyPressed(int keyCode)
    {
        if (keyMap.find(keyCode) == keyMap.end())
        {
            return;
        }

        *keyMap[keyCode] = true;
    }

    void KeyReleased(int keyCode)
    {
        if (keyMap.find(keyCode) == keyMap.end())
        {
            return;
        }

        *keyMap[keyCode] = false;
    }

  private:
    void setKeyVariables(uint8_t amount, int8_t& key) { key = amount; }
    int8_t up, down, left, right;
    std::unordered_map<int, int8_t*> keyMap;
    uint32_t moveSeq;
};

// TODO: global input state for now until we decide what to do with it
static auto sInputButtonState = InputButtonState();

void LoadTextures()
{
    SpriteSheetData::RegisterSpriteSheetData(gameobjects::PlayerSheetKey, TESTANIMATEDSHEET,
                                             TESTANIMATEDDATA);
}

bool SetupRenderer()
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
    LoadTextures();
    return true;
}

bool DoFrame(uint32_t currentTime)
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

    // TODO: Maybe do this here? Probably should do the render stuff
    // in a separate function
    sInputButtonState.QueueInputEvents();

    // TODO: Add timing
    EventManager::sInstance->FireEvents(10);

    RenderManager::sInstance->Render();

    NetworkManagerClient::sInstance->SendOutgoingPackets();
    NetworkManagerClient::sInstance->packetManager.StepTime(currentTime);
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
    AddMessageCtor(messageSerializer, CreatePlayerOwnedObject);
    AddMessageCtor(messageSerializer, PhysicsComponentUpdate);

    auto packetSerializer = std::make_shared<PacketSerializer>(messageSerializer);
    AddPacketCtor(packetSerializer, ReliableOrderedPacket);
    AddPacketCtor(packetSerializer, UnauthenticatedPacket);
    AddPacketCtor(packetSerializer, AuthenticatedPacket);

    NetworkManagerClient::StaticInit(serverDestination, packetSerializer, "Joe Mamma");
}

void SetupWorld()
{
    EventManager::StaticInit();

    gameobjects::WorldClient::StaticInit();
    gameobjects::SetupLogger(logger::level::INFO);

    // create registry and add all the creation functions we know about
    gameobjects::Registry::StaticInit(gameobjects::WorldClient::StaticAddGameObject);
    gameobjects::Registry::sInstance->RegisterCreationFunction(
        gameobjects::PLAYER_ID, gameobjects::PlayerClient::StaticCreate);

    // World listens for requests to add objects
    auto addObject = CREATE_DELEGATE(&gameobjects::WorldClient::OnAddObject,
                                     gameobjects::WorldClient::sInstance);
    auto stateUpdate = CREATE_DELEGATE(&gameobjects::WorldClient::OnStateUpdateMessage,
                                       gameobjects::WorldClient::sInstance);

    EventManager::sInstance->AddListener(addObject, CreatePlayerOwnedObject::EVENT_TYPE);
    EventManager::sInstance->AddListener(stateUpdate, PhysicsComponentUpdate::EVENT_TYPE);
}

void initStuffs()
{
    logger::InitLog(logger::level::TRACE, "Main Logger");
    if (!SetupRenderer())
    {
        ERROR("Can't initialize sdl, bailing");
        SDL_Quit();
    };

    DEBUG("Starting Client")

    std::string destination = "127.0.0.1:4500";

    SetupNetworking(destination);
    SetupWorld();

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
    InputManager::sInstance->RegisterKeyDownListner(
        [](int keyCode) { sInputButtonState.KeyPressed(keyCode); });
    InputManager::sInstance->RegisterKeyUpListner(
        [](int keyCode) { sInputButtonState.KeyReleased(keyCode); });
}

int main(int argc, const char* argv[])
{
    __argc = argc;
    __argv = argv;

    Engine engine = Engine(initStuffs, DoFrame);
    engine.Run();
}
