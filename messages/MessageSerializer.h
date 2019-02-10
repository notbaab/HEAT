#pragma once
#include <unordered_map>

class InputMemoryBitStream;
class OutputMemoryBitStream;
class Message;
using MessageConstructor = std::function<std::unique_ptr<Message>()>;

// A lambda for instantiating the default Message constructor and returning ptr to it
#define MessageCtor(messageType)                                                                   \
    []() -> std::unique_ptr<Message> { return std::move(std::make_unique<messageType>()); }

// Cleaner wrapper around adding the constructor by using the expected static ID
#define AddMessageCtor(serializer, messageType)                                                    \
    serializer.AddConstructor(messageType::ID, MessageCtor(messageType))

class MessageSerializer
{
  public:
    MessageSerializer(){};
    ~MessageSerializer(){};
    std::unordered_map<uint32_t, MessageConstructor> messageConstructors;

    bool AddConstructor(uint32_t id, MessageConstructor constructor);
    std::unique_ptr<Message> CreateMessage(uint32_t id);
    std::vector<std::unique_ptr<Message>> ReadMessages(InputMemoryBitStream& in);
    bool WriteMessages(std::vector<std::unique_ptr<Message>>& packets, OutputMemoryBitStream& out);
};
