/**
 * Just a GUI that takes in either messages or a message file and
 * plays them back. Meant to either be a view into what the server
 * sees or to playback a stream of messages. Meant to be pretty much
 * a whole client without a networking component
 */

#include <memory>
#include <unordered_map>

// Yeah this is too many includes
#include "IO/InputMemoryBitStream.h"
#include "IO/OutputMemoryBitStream.h"
#include "debugging_tools/debug_commands.h"
#include "debugging_tools/debug_socket.h"
#include "debugging_tools/debug_tools.h"
#include "dvr/DVR.h"
#include "engine/Engine.h"
#include "events/CreatePlayerOwnedObject.h"
#include "events/EventManager.h"
#include "events/EventRouter.h"
#include "events/LoggedIn.h"
#include "events/PhysicsComponentUpdate.h"
#include "events/PlayerInputEvent.h"
#include "events/RemoveClientOwnedGameObjectsEvent.h"
#include "gameobjects/PlayerClient.h"
#include "gameobjects/Registry.h"
#include "gameobjects/SetupGameObjects.h"
#include "gameobjects/WorldClient.h"
#include "graphics/TiledTileLoader.h"
#include "graphics/WindowManager.h"
#include "managers/NetworkManagerClient.h"
#include "managers/NullNetworkManagerClient.h"
#include "messages/ClientConnectionChallengeMessage.h"
#include "packets/AuthenticatedPacket.h"

#define ASSET_MAP "images/asset-map.json"

static std::shared_ptr<Engine> clientInstance;
static std::string playbackFile;

std::string DVRReplayReceivedMessages(std::vector<std::string> args)
{
    if (args.size() != 1)
    {
        return "Must give one argument";
    }

    return "Did something";
}

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
    auto frameMessages = DVR::sInstance->PopRecvMessages(currentTime);

    TRACE("Frame Message Count {}", frameMessages.size());
    for (auto& msg : frameMessages)
    {
        if (msg.message == nullptr)
        {
            TRACE("AHAlH here");
        }
        NetworkManagerClient::sInstance->HandleMessage(msg.message);
    }

    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            SDL_Quit();
            return false;
        }
    }

    // TODO: Add timing
    EventManager::sInstance->FireEvents(currentTime);
    gameobjects::WorldClient::sInstance->Update(currentTime);

    RenderManager::sInstance->Render();
    INFO("out");

    // networkManager->SendOutgoingPackets();
    // networkManager->Tick(currentTime);

    return true;
}

void SetupWorld()
{
    EventManager::StaticInit();

    gameobjects::WorldClient::StaticInit();
    gameobjects::SetupLogger(logger::level::TRACE);

    // create registry and add all the creation functions we know about
    gameobjects::Registry::StaticInit(gameobjects::WorldClient::StaticAddGameObject);
    gameobjects::Registry::sInstance->RegisterCreationFunction(gameobjects::PLAYER_ID,
                                                               gameobjects::PlayerClient::StaticCreate);

    // World listens for requests to add objects
    auto addObject = CREATE_DELEGATE_LAMBDA(gameobjects::WorldClient::sInstance->OnAddObject);
    EventManager::sInstance->AddListener(addObject, CreatePlayerOwnedObject::EVENT_TYPE);

    auto stateUpdate = CREATE_DELEGATE_LAMBDA(gameobjects::WorldClient::sInstance->OnStateUpdateMessage);
    EventManager::sInstance->AddListener(stateUpdate, PhysicsComponentUpdate::EVENT_TYPE);

    auto removeClientObjects =
        CREATE_DELEGATE(&gameobjects::World::OnRemoveClientOwnedObjects, gameobjects::WorldClient::sInstance);
    EventManager::sInstance->AddListener(removeClientObjects, RemoveClientOwnedGameObjectsEvent::EVENT_TYPE);

    // Router for player events
    EventRouter<PlayerInputEvent>::StaticInit();
    auto playerInputRouter = CREATE_DELEGATE_LAMBDA((EventRouter<PlayerInputEvent>::sInstance->RouteEvent));
    EventManager::sInstance->AddListener(playerInputRouter, PlayerInputEvent::EVENT_TYPE);
}

static void SetupDebugTools() { InitDebugingTools(); }

void initStuffs()
{
    logger::InitLog(logger::DEBUG, "Main Logger");
    if (!SetupRenderer())
    {
        ERROR("Can't initialize sdl, bailing");
        SDL_Quit();
    };

    INFO("Starting Viewer");

    std::string destination = "127.0.0.1:4500";

    // We don't need these but we are using the network manager instead of the
    // event system directly. I'm not sure if I like that.
    auto messageSerializer = std::make_shared<MessageSerializer>();
    auto bitReader = std::make_unique<InputMemoryBitStream>();
    auto bitWriter = std::make_unique<OutputMemoryBitStream>();
    auto packetReader = std::make_unique<StructuredDataReader>(std::move(bitReader));
    auto packetWriter = std::make_unique<StructuredDataWriter>(std::move(bitWriter));

    auto packetSerializer =
        std::make_shared<PacketSerializer>(messageSerializer, std::move(packetReader), std::move(packetWriter));

    NetworkManagerClient::StaticInit(destination, packetSerializer, "Joe Mamma");

    SetupDebugTools();
    SetupWorld();
    // DVR Playback
    DVR::StaticInit();
    DVR::sInstance->ReadReceivedPacketsFromFile(playbackFile);
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

        if (!strcmp(*argv, "--playback-recording"))
        {
            argv++;
            argc--;
            playbackFile = *argv;
            INFO("Setting playback file to {}", playbackFile);
        }
    }

    clientInstance = std::make_shared<Engine>(initStuffs, DoFrame);
    clientInstance->Run();
}
