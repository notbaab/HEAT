#pragma once
#include <unordered_map>

class InputMemoryBitStream;
class OutputMemoryBitStream;
class Packet;
using PacketConstructor = std::function<std::unique_ptr<Packet>()>;

// A lambda for instantiating the default packet constructor and returning ptr to it
#define PacketCtor(packetType)                                                                     \
    []() -> std::unique_ptr<Packet> { return std::move(std::make_unique<packetType>()); }

// Cleaner wrapper around adding the constructor by using the expected static ID
#define AddPacketCtor(serializer, packetType)                                                      \
    serializer.AddConstructor(packetType::ID, PacketCtor(packetType))

class PacketSerializer
{
  public:
    PacketSerializer(){};
    ~PacketSerializer(){};
    std::unordered_map<uint32_t, PacketConstructor> packetConstructors;

    bool AddConstructor(uint32_t id, PacketConstructor constructor);
    std::unique_ptr<Packet> CreatePacket(uint32_t id);
    std::vector<std::unique_ptr<Packet>> ReadPackets(InputMemoryBitStream& in);
    bool WritePackets(std::vector<std::unique_ptr<Packet>>& packets, OutputMemoryBitStream& out);
};
