#include "MessageSerializer.h"
#include "InputMemoryBitStream.h"
#include "Message.h"
#include "OutputMemoryBitStream.h"

// TODO: This and Packet serializer could probably be a templated class since they
// do the exact same thing. If they don't diverge any further do that.
bool MessageSerializer::AddConstructor(uint32_t id, MessageConstructor constructor)
{
    if (messageConstructors.find(id) != messageConstructors.end())
    {
        return false;
    }

    messageConstructors[id] = constructor;
    return true;
}

std::unique_ptr<Message> MessageSerializer::CreateMessage(uint32_t id)
{
    if (messageConstructors.find(id) != messageConstructors.end())
    {
        return NULL;
    }

    return messageConstructors[id]();
}

std::vector<std::unique_ptr<Message>> MessageSerializer::ReadMessages(InputMemoryBitStream& in)
{
    uint32_t id;

    std::vector<std::unique_ptr<Message>> messages;

    while (in.GetRemainingBitCount() > 0)
    {
        in.serialize(id);
        if (messageConstructors.find(id) == messageConstructors.end())
        {
            throw std::runtime_error("Bad message data!!!!");
        }

        std::unique_ptr<Message> message = messageConstructors[id]();
        message->Read(in);
        messages.push_back(std::move(message));
    }

    return messages;
}

bool MessageSerializer::WriteMessages(std::vector<std::unique_ptr<Message>>& messages,
                                      OutputMemoryBitStream& out)
{
    for (auto const& message : messages)
    {
        uint32_t id = message->GetUniqueId();
        out.Write(id);
        message->Write(out);
    }

    return true;
}
