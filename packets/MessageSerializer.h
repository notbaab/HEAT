#pragma once
#include <functional>
#include <memory>
#include <unordered_map>

#include "IO/StructuredDataReader.h"
#include "Message.h"

class InputMemoryBitStream;
class OutputMemoryBitStream;

using MessageConstructor = std::function<std::unique_ptr<Message>()>;

// A lambda for instantiating the default Message constructor and returning ptr to it
#define MessageCtor(messageType) []() -> std::unique_ptr<Message> { return std::move(std::make_unique<messageType>()); }

// Cleaner wrapper around adding the constructor by using the expected static ID
#define AddMessageCtor(serializer, messageType)                                                                        \
    serializer->AddConstructor(messageType::CLASS_ID, MessageCtor(messageType))

class MessageSerializer
{
  public:
    MessageSerializer() {}
    ~MessageSerializer() {}
    std::unordered_map<uint32_t, MessageConstructor> messageConstructors;

    bool AddConstructor(uint32_t id, MessageConstructor constructor);
    std::unique_ptr<Message> CreateMessage(uint32_t id);

    std::shared_ptr<std::vector<std::shared_ptr<Message>>> ReadMessages(StructuredDataReader& reader,
                                                                        uint8_t numMessages)
    {
        uint32_t id;

        auto messages = std::make_unique<std::vector<std::shared_ptr<Message>>>();
        uint8_t remainingMessages = numMessages;

        while (remainingMessages > 0)
        {
            reader.StartObject();

            reader.serialize(id, "id");

            auto identifier = Message::StringFromId(id);
            if (messageConstructors.find(id) == messageConstructors.end())
            {
                std::cout << "No message constructor for type " << identifier << std::endl;
                // TODO: So we just bomb out here? Maybe a little less...dramatic would be better. Or
                // configure it
                throw std::runtime_error("Bad message data!!!!");
            }

            std::shared_ptr<Message> message = messageConstructors[id]();
            message->Read(reader);
            reader.EndObject();

            // stream.endObject()
            messages->push_back(std::move(message));
            remainingMessages--;
        }

        return std::move(messages);
    }

    template <typename T>
    bool WriteMessages(std::shared_ptr<std::vector<std::shared_ptr<Message>>> messages, T& out)
    {
        for (auto const& message : *messages)
        {
            uint32_t id = message->GetClassIdentifier();
            auto identifier = Message::StringFromId(id);
            out.StartObject();
            out.serialize(id, "id");
            message->Write(out);
            out.EndObject();
        }

        return true;
    }
};
