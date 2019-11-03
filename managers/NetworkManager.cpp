#include <random>
#include <stdlib.h>

#include "IO/InputMemoryBitStream.h"
#include "NetworkManager.h"
#include "PacketManager.h"
#include "packets/PacketSerializer.h"

uint64_t GenerateSalt()
{
    return ((uint64_t(rand()) << 0) & 0x000000000000FFFFull) |
           ((uint64_t(rand()) << 16) & 0x00000000FFFF0000ull) |
           ((uint64_t(rand()) << 32) & 0x0000FFFF00000000ull) |
           ((uint64_t(rand()) << 48) & 0xFFFF000000000000ull);
}

// Bind our data received callback function
NetworkManager::NetworkManager(uint16_t port, std::shared_ptr<PacketSerializer> packetSerializer)
    : socketManager(SocketManager(port, std::bind(&NetworkManager::dataRecievedCallback, this,
                                                  std::placeholders::_1, std::placeholders::_2)))
{
    this->packetSerializer = packetSerializer;
}

NetworkManager::NetworkManager(std::shared_ptr<PacketSerializer> packetSerializer)
    : socketManager(SocketManager(std::bind(&NetworkManager::dataRecievedCallback, this,
                                            std::placeholders::_1, std::placeholders::_2)))
{
    this->packetSerializer = packetSerializer;
}

void NetworkManager::dataRecievedCallback(SocketAddress fromAddress,
                                          std::unique_ptr<std::vector<uint8_t>> data)
{
    // This will be a bit wonky with our setup. So we need to see if
    // we have a packet manager yet for this from address key. If so,
    // let it handle reading the packet and such. If not, we need to create
    // it and store in the "new connection state" list.
    auto packets = packetSerializer->ReadPackets(std::move(data));

    for (auto const& packet : packets)
    {
        // Should I guard this with some sort of class check?
        auto cast = std::static_pointer_cast<ReliableOrderedPacket>(packet);
    }
}
