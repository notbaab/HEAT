#pragma once
#include <unordered_map>

class InputMemoryBitStream;
class OutputMemoryBitStream;
class Packet;
using PacketConstructor = std::function<std::shared_ptr<Packet>()>;

// A lambda for instantiating the default packet constructor and returning ptr to it
#define PacketCtor(packetType)                                                                     \
    []() -> std::shared_ptr<Packet> { return std::make_shared<packetType>(); }

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
    std::vector<std::shared_ptr<Packet>> ReadPackets(InputMemoryBitStream& in);
    bool WritePackets(std::vector<std::shared_ptr<Packet>> packets, OutputMemoryBitStream& out);
};
