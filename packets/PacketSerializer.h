#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

class StructuredDataReader;
class StructuredDataWriter;
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
    PacketSerializer(std::shared_ptr<MessageSerializer> messageFactory, std::unique_ptr<StructuredDataReader> reader,
                     std::unique_ptr<StructuredDataWriter> writer)
        : messageFactory(messageFactory), reader(std::move(reader)), writer(std::move(writer))
    {
    }

    ~PacketSerializer() {}
    std::unordered_map<uint32_t, PacketConstructor> packetConstructors;

    bool AddConstructor(uint32_t id, PacketConstructor constructor);
    std::shared_ptr<Packet> CreatePacket(uint32_t id);

    std::vector<std::shared_ptr<Packet>> ReadPackets(std::unique_ptr<std::vector<uint8_t>> data);

    // bool WritePackets(std::vector<std::shared_ptr<Packet>>& packets, uint8_t* outBuff, uint32_t* outSize);
    bool WritePacket(std::shared_ptr<Packet> packet, const uint8_t** outBuff, uint32_t* outSize);

  private:
    std::shared_ptr<MessageSerializer> messageFactory;

    std::unique_ptr<StructuredDataReader> reader;
    std::unique_ptr<StructuredDataWriter> writer;
};
