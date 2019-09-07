#include "NetworkManager.h"
#include "IO/InputMemoryBitStream.h"
#include "PacketManager.h"

// Bind our data received callback function
NetworkManager::NetworkManager(uint16_t port, std::shared_ptr<PacketManager> packetManager)
    : socketManager(SocketManager(port, std::bind(&NetworkManager::dataRecievedCallback, this,
                                                  std::placeholders::_1, std::placeholders::_2)))
{
    this->packetManager = packetManager;
}

void NetworkManager::dataRecievedCallback(uint64_t fromAddressKey,
                                          std::unique_ptr<std::vector<uint8_t>> data)
{
    auto packets = packetManager->ConvertBytesToPackets(std::move(data));

    for (auto const& packet : packets)
    {
        // Should I guard this with some sort of class check?
        auto cast = std::static_pointer_cast<ReliableOrderedPacket>(packet);
        bool weGood = packetManager->ReadPacket(cast);

        if (!weGood)
        {
            std::cout << "well fuck me" << std::endl;
        }
        else
        {
            std::cout << "Damn son, we got that shit" << std::endl;
        }
    }
}
