#include <iostream>
#include <string>
#include <thread>

#include "managers/NetworkManagerServer.h"
#include "managers/PacketManager.h"
#include "messages/PlayerMessage.h"
#include "networking/SocketManager.h"

#define EXIT "exit"

// Global thing until I figure out how I want to do management stuffs.
// Maybe like a engine or something
std::shared_ptr<PacketManager> packetManager;

void callback(std::unique_ptr<std::vector<uint8_t>> data)
{

    std::shared_ptr<std::vector<uint8_t>> d = std::move(data);
    networking::printData(d);

    auto stream = InputMemoryBitStream(d);

    // TODO: this is a little odd. The serialize is in the manager,
    // probably should just be able to pass the stream to the manager
    auto packets = packetManager->m_packetFactory->ReadPackets(stream);
    for (auto const& packet : packets)
    {
        // Should I gaurd this with some sort of class check?
        auto cast = std::static_pointer_cast<ReliableOrderedPacket>(packet);
        bool weGood = packetManager->ReadPacket(cast);

        if (!weGood)
        {
            std::cout << "well fuck me" << std::endl;
        }
    }
}

const char** __argv;
int __argc;
int main(int argc, const char* argv[])
{
    // NetworkManager Setup
    auto messageSerializer = std::make_shared<MessageSerializer>();
    AddMessageCtor(messageSerializer, PlayerMessage);
    auto packetSerializer = std::make_shared<PacketSerializer>(messageSerializer);
    AddPacketCtor(packetSerializer, ReliableOrderedPacket);
    packetManager = std::make_shared<PacketManager>(packetSerializer);

    auto nm = NetworkManagerServer(4500, packetManager);
    // auto socketManager = SocketManager(4500, callback);
    usleep(10 * 1000 * 1000);
}
