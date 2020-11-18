#include "HNetworkManager.h"

namespace holistic
{

uint64_t GenerateSalt()
{
    return ((uint64_t(rand()) << 0) & 0x000000000000FFFFull) | ((uint64_t(rand()) << 16) & 0x00000000FFFF0000ull) |
           ((uint64_t(rand()) << 32) & 0x0000FFFF00000000ull) | ((uint64_t(rand()) << 48) & 0xFFFF000000000000ull);
}

HNetworkManager::HNetworkManager(uint16_t port, std::shared_ptr<PacketSerializer> packetSerializer)
    : socketManager(SocketManager(
          port, std::bind(&HNetworkManager::DataReceivedCallback, this, std::placeholders::_1, std::placeholders::_2)))
{
    this->packetSerializer = packetSerializer;
}

HNetworkManager::HNetworkManager(std::shared_ptr<PacketSerializer> packetSerializer)
    : socketManager(SocketManager(
          std::bind(&HNetworkManager::DataReceivedCallback, this, std::placeholders::_1, std::placeholders::_2)))
{
    this->packetSerializer = packetSerializer;
}

} // namespace holistic
