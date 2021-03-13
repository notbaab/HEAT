#include <unordered_map>

// Yeah this is too many includes
#include "IO/InputMemoryBitStream.h"
#include "IO/JsonInputMemoryStream.h"
#include "IO/JsonOutputMemoryStream.h"
#include "IO/OutputMemoryBitStream.h"
#include "controls/InputManager.h"
#include "debugging_tools/debug_commands.h"
#include "debugging_tools/debug_socket.h"
#include "debugging_tools/debug_tools.h"
#include "dvr/DVR.h"
#include "dvr/DVRConsoleCommands.cpp"
#include "engine/Engine.h"
#include "engine/ServiceLocator.h"
#include "events/CreatePlayerOwnedObject.h"
#include "events/EventManager.h"
#include "events/EventRouter.h"
#include "events/LoggedIn.h"
#include "events/PhysicsComponentUpdate.h"
#include "events/PlayerInputEvent.h"
#include "events/RemoveClientOwnedGameObjectsEvent.h"
#include "events/RemoveGameObjectEvent.h"
#include "gameobjects/PlayerClient.h"
#include "gameobjects/Registry.h"
#include "gameobjects/SetupGameObjects.h"
#include "gameobjects/WorldClient.h"
#include "graphics/TiledTileLoader.h"
#include "graphics/WindowManager.h"
#include "managers/NetworkManagerClient.h"
#include "managers/NullNetworkManagerClient.h"
#include "messages/ClientConnectionChallengeMessage.h"
#include "messages/ClientLoginMessage.h"
#include "messages/ClientLoginResponse.h"
#include "messages/PlayerMessage.h"
#include "packets/AuthenticatedPacket.h"
#include "packets/UnauthenticatedPacket.h"

static bool noServer = false;
static bool playingBackMessages = false;

#define ASSET_MAP "images/asset-map.json"

static std::shared_ptr<Engine> clientInstance;

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
            // Skip 0
            sendMove();
            sentStopInput = false;
        }
        else if (!sentStopInput)
        {
            sentStopInput = true;
            sendMove();
        }
    }

  private:
    void sendMove()
    {
        auto networkManager = ServiceLocator::GetNetworkManager<NetworkManagerClient*>();

        moveSeq++;

        auto clientId = gameobjects::PlayerClient::localPlayerClientId;
        auto playerObjId = gameobjects::PlayerClient::localPlayerId;

        auto inputState =
            std::make_shared<PlayerInputEvent>(GetHorizontalDirection(), GetVerticalDirection(), moveSeq, playerObjId);
        EventManager::sInstance->QueueEvent(inputState);
    }

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
    bool sentStopInput = true;
};

// TODO: global input state for now until we decide what to do with it
static auto sInputButtonState = InputButtonState();

void LoadTextures()
{
    AssetManager::StaticInit();

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
    auto networkManager = ServiceLocator::GetNetworkManager<NetworkManagerClient*>();
    auto dvr = DVR::sInstance.get();

    networkManager->ProcessMessages();

    // If we are playing messages, grab them here and feed them in the networkManager
    // if (dvr->playingBack)
    // {
    // }

    SDL_Event event;

    // clear out the event queue
    memset(&event, 0, sizeof(SDL_Event));
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            SDL_Quit();
            return false;
        }
        else
        {
            InputManager::sInstance->HandleSDLEvent(event);
        }
    }

    // TODO: Maybe do this here? Probably should do the render stuff
    // in a separate function
    sInputButtonState.QueueInputEvent();

    // TODO: Add timing
    EventManager::sInstance->FireEvents(currentTime);
    gameobjects::WorldClient::sInstance->Update(currentTime);

    RenderManager::sInstance->Render();

    networkManager->SendOutgoingPackets();
    networkManager->Tick(currentTime);

    return true;
}

void SetupNetworking(std::string serverDestination)
{
    if (noServer)
    {
        NullNetworkManagerClient::StaticInit();

        // Well. Alright then, so much for unique ptr providing some safety
        ServiceLocator::Provide(NullNetworkManagerClient::sInstance.get());
        return;
    }

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
    AddMessageCtor(messageSerializer, RemoveGameObjectEvent);
    AddMessageCtor(messageSerializer, RemoveClientOwnedGameObjectsEvent);

    auto bitReader = std::make_unique<InputMemoryBitStream>();
    auto bitWriter = std::make_unique<OutputMemoryBitStream>();
    auto packetReader = std::make_unique<StructuredDataReader>(std::move(bitReader));
    auto packetWriter = std::make_unique<StructuredDataWriter>(std::move(bitWriter));

    auto packetSerializer =
        std::make_shared<PacketSerializer>(messageSerializer, std::move(packetReader), std::move(packetWriter));
    AddPacketCtor(packetSerializer, ReliableOrderedPacket);
    AddPacketCtor(packetSerializer, UnauthenticatedPacket);
    AddPacketCtor(packetSerializer, AuthenticatedPacket);

    NetworkManagerClient::StaticInit(serverDestination, packetSerializer, "Joe Mamma");

    // Well. Alright then, so much for unique ptr providing some safety
    ServiceLocator::Provide(NetworkManagerClient::sInstance.get());
}

void SetupWorld()
{
    auto networkManager = ServiceLocator::GetNetworkManager<NetworkManagerClient*>();
    // Need to be called after the service locater has provided a network manager client
    assert(networkManager);

    EventManager::StaticInit();

    gameobjects::WorldClient::StaticInit();
    gameobjects::SetupLogger(logger::level::DEBUG);

    // create registry and add all the creation functions we know about
    gameobjects::Registry::StaticInit(gameobjects::WorldClient::StaticAddGameObject);
    gameobjects::Registry::sInstance->RegisterCreationFunction(gameobjects::PLAYER_ID,
                                                               gameobjects::PlayerClient::StaticCreate);

    // World listens for requests to add objects
    auto addObject = CREATE_DELEGATE_LAMBDA(gameobjects::WorldClient::sInstance->OnAddObject);
    EventManager::sInstance->AddListener(addObject, CreatePlayerOwnedObject::EVENT_TYPE);

    auto stateUpdate = CREATE_DELEGATE_LAMBDA(gameobjects::WorldClient::sInstance->OnStateUpdateMessage);
    EventManager::sInstance->AddListener(stateUpdate, PhysicsComponentUpdate::EVENT_TYPE);

    auto loggedIn = CREATE_DELEGATE_LAMBDA(gameobjects::PlayerClient::UserLoggedIn);
    EventManager::sInstance->AddListener(loggedIn, LoggedIn::EVENT_TYPE);

    auto removeClientObjects =
        CREATE_DELEGATE(&gameobjects::World::OnRemoveClientOwnedObjects, gameobjects::WorldClient::sInstance);
    EventManager::sInstance->AddListener(removeClientObjects, RemoveClientOwnedGameObjectsEvent::EVENT_TYPE);

    // TODO: This means we cannot change the network manager while running. We should find a way to signal that
    auto eventForwarder = CREATE_DELEGATE_LAMBDA_CAPTURE_BY_VALUE(networkManager, QueueMessage);
    EventManager::sInstance->AddListener(eventForwarder, PlayerInputEvent::EVENT_TYPE);

    // Router for player events
    EventRouter<PlayerInputEvent>::StaticInit();
    auto playerInputRouter = CREATE_DELEGATE_LAMBDA((EventRouter<PlayerInputEvent>::sInstance->RouteEvent));
    EventManager::sInstance->AddListener(playerInputRouter, PlayerInputEvent::EVENT_TYPE);
}

static void SetupDebugTools()
{
    InitDebugingTools();

    add_command("dvr-get-recv-packets", DVRGetRecvPackets);
    add_command("dvr-get-recv-messages", DVRGetRecvMessages);
    add_command("dvr-write-messages", DVRWriteMessages);
    add_command("dvr-replay-packets", DVRReplayPackets);
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
    SetupDebugTools();

    SetupNetworking(destination);
    SetupWorld();

    // DVR Recording
    DVR::StaticInit();
    auto recievedPacket = CREATE_DELEGATE(&DVR::PacketReceived, DVR::sInstance);
    EventManager::sInstance->AddListener(recievedPacket, PacketReceivedEvent::EVENT_TYPE);

    auto networkManager = ServiceLocator::GetNetworkManager<NetworkManagerClient*>();

    // Start the handshake asap and force the packets to be sent
    networkManager->StartServerHandshake();
    networkManager->SendOutgoingPackets();

    InputManager::StaticInit();
    InputManager::sInstance->RegisterKeyDownListner([](int keyCode) { sInputButtonState.KeyPressed(keyCode); });
    InputManager::sInstance->RegisterKeyUpListner([](int keyCode) { sInputButtonState.KeyReleased(keyCode); });
}

int main(int argc, const char* argv[])
{
    while (argc > 1)
    {
        argc--;
        argv++;
        if (!strcmp(*argv, "--debug-socket"))
        {
            argv++;
            argc--;
            const char* socketPath = *argv;
            INFO("Starting a debug socket at {}", socketPath);
            SpawnSocket(socketPath, DebugCommandHandler);
        }
    }

    clientInstance.reset(new Engine(initStuffs, DoFrame));
    clientInstance->Run();
}
