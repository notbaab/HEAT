#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

class InputMemoryBitStream;
class OutputMemoryBitStream;
class Packet;
class MessageSerializer;

using PacketConstructor = std::function<std::shared_ptr<Packet>(std::shared_ptr<MessageSerializer>)>;
// using PacketConstructor = std::function<std::shared_ptr<Packet>()>;

// A lambda for instantiating the default packet constructor and returning ptr to it
#define PacketCtor(packetType)                                                                                         \
    [](auto messageFactory) -> std::shared_ptr<Packet> {                                                               \
        return std::move(std::make_unique<packetType>(messageFactory));                                                \
    }

// Cleaner wrapper around adding the constructor by using the expected static ID
#define AddPacketCtor(serializer, packetType) serializer->AddConstructor(packetType::CLASS_ID, PacketCtor(packetType))

class PacketSerializer
{
  public:
    PacketSerializer(std::shared_ptr<MessageSerializer> messageFactory) : messageFactory(messageFactory) {}
    ~PacketSerializer() {}
    std::unordered_map<uint32_t, PacketConstructor> packetConstructors;

    bool AddConstructor(uint32_t id, PacketConstructor constructor);
    std::shared_ptr<Packet> CreatePacket(uint32_t id);

    std::vector<std::shared_ptr<Packet>> ReadPackets(InputMemoryBitStream& in);
    std::vector<std::shared_ptr<Packet>> ReadPackets(std::unique_ptr<std::vector<uint8_t>> data);

    bool WritePackets(std::vector<std::shared_ptr<Packet>>& packets, OutputMemoryBitStream& out);
    bool WritePacket(std::shared_ptr<Packet> packet, OutputMemoryBitStream& out);

  private:
    std::shared_ptr<MessageSerializer> messageFactory;
};
