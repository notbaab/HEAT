#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>

// Yeah this is too many includes
#include "controls/InputManager.h"
#include "engine/Engine.h"
#include "events/CreatePlayerOwnedObject.h"
#include "events/Event.h"
#include "events/EventManager.h"
#include "events/EventRouter.h"
#include "events/PhysicsComponentUpdate.h"
#include "events/PlayerInputEvent.h"
#include "gameobjects/PlayerClient.h"
#include "gameobjects/Registry.h"
#include "gameobjects/SetupGameObjects.h"
#include "gameobjects/SimpleGameObject.h"
#include "gameobjects/WorldClient.h"
#include "graphics/AnimatedSpriteComponent.h"
#include "graphics/AssetManager.h"
#include "graphics/GraphicsDriver.h"
#include "graphics/RenderManager.h"
#include "graphics/StaticSpriteComponent.h"
#include "graphics/TiledAnimatedSpriteSheetData.h"
#include "graphics/TiledTileLoader.h"
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

#define ASSET_MAP "images/asset-map.json"

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
    int8_t GetHorizontalDirection() const { return right - left; };
    int8_t GetVerticalDirection() const { return down - up; };
    void KeyPressed(int keyCode) { setKeyVariables(keyCode, true); }
    void KeyReleased(int keyCode) { setKeyVariables(keyCode, false); }

    // if we have input, make a PlayerInputState and queue it into the events
    void QueueInputEvent()
    {
        if (hasInput())
        {
            // TODO: Don't grab it, just don't initialize the player input stuff until a player is
            // added.
            // Skip 0
            moveSeq++;
            auto clientId = NetworkManagerClient::GetClientId();
            auto playerObjId = gameobjects::PlayerClient::clientToPlayer[clientId];
            auto inputState = std::make_shared<PlayerInputEvent>(GetHorizontalDirection(), GetVerticalDirection(),
                                                                 moveSeq, playerObjId);
            EventManager::sInstance->QueueEvent(inputState);
        }
    }

  private:
    void setKeyVariables(int keyCode, bool value)
    {
        if (keyMap.find(keyCode) == keyMap.end())
        {
            INFO("No key for {}", keyCode);
            return;
        }

        *keyMap[keyCode] = value;
    }

    int8_t up, down, left, right;
    std::unordered_map<int, int8_t*> keyMap;
    uint32_t moveSeq;
};

// TODO: global input state for now until we decide what to do with it
static auto sInputButtonState = InputButtonState();

void LoadTextures()
{
    auto loaded = AssetManager::StaticInit(ASSET_MAP);

    TiledTileLoader t = TiledTileLoader();
    auto sheetData = t.LoadAnimationSheetInfo("images/player.json");
    AssetManager::sInstance->PushAnimatedTiledSheet(std::move(sheetData), "images/chara_hero.png");
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
    sInputButtonState.QueueInputEvent();

    // TODO: Add timing
    EventManager::sInstance->FireEvents(currentTime);
    gameobjects::WorldClient::sInstance->Update(currentTime);

    RenderManager::sInstance->Render();

    NetworkManagerClient::sInstance->SendOutgoingPackets();
    NetworkManagerClient::sInstance->packetManager.SetTime(currentTime);
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
    AddMessageCtor(messageSerializer, PlayerInputEvent);

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
    gameobjects::SetupLogger(logger::level::DEBUG);

    // create registry and add all the creation functions we know about
    gameobjects::Registry::StaticInit(gameobjects::WorldClient::StaticAddGameObject);
    gameobjects::Registry::sInstance->RegisterCreationFunction(gameobjects::PLAYER_ID,
                                                               gameobjects::PlayerClient::StaticCreate);

    // World listens for requests to add objects
    auto addObject = CREATE_DELEGATE_LAMBDA(gameobjects::WorldClient::sInstance->OnAddObject);
    auto stateUpdate = CREATE_DELEGATE_LAMBDA(gameobjects::WorldClient::sInstance->OnStateUpdateMessage);
    auto eventForwarder = CREATE_DELEGATE_LAMBDA(NetworkManagerClient::sInstance->QueueMessage);

    EventManager::sInstance->AddListener(addObject, CreatePlayerOwnedObject::EVENT_TYPE);
    EventManager::sInstance->AddListener(stateUpdate, PhysicsComponentUpdate::EVENT_TYPE);
    EventManager::sInstance->AddListener(eventForwarder, PlayerInputEvent::EVENT_TYPE);

    // Router for player events
    EventRouter<PlayerInputEvent>::StaticInit();
    // auto playerInputRouter = CREATE_DELEGATE_LAMBDA(
    //     (EventRouter<gameobjects::PlayerClient, PlayerInputEvent>::sInstance->RouteEvent));
    auto playerInputRouter = CREATE_DELEGATE_LAMBDA((EventRouter<PlayerInputEvent>::sInstance->RouteEvent));
    EventManager::sInstance->AddListener(playerInputRouter, PlayerInputEvent::EVENT_TYPE);
}

void initStuffs()
{
    logger::InitLog(logger::DEBUG, "Main Logger");
    if (!SetupRenderer())
    {
        ERROR("Can't initialize sdl, bailing");
        SDL_Quit();
    };

    INFO("Starting Client");

    std::string destination = "127.0.0.1:4500";

    SetupNetworking(destination);
    SetupWorld();

    NetworkManagerClient::sInstance->StartServerHandshake();
    NetworkManagerClient::sInstance->SendOutgoingPackets();

    InputManager::StaticInit();
    InputManager::sInstance->RegisterKeyDownListner([](int keyCode) { sInputButtonState.KeyPressed(keyCode); });
    InputManager::sInstance->RegisterKeyUpListner([](int keyCode) { sInputButtonState.KeyReleased(keyCode); });
}

int main(int argc, const char* argv[])
{
    __argc = argc;
    __argv = argv;

    Engine engine = Engine(initStuffs, DoFrame);
    engine.Run();
}
