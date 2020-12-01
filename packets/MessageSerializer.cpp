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
        // already added, maybe log so?
        return false;
    }

    messageConstructors[id] = constructor;
    return true;
}

std::unique_ptr<Message> MessageSerializer::CreateMessage(uint32_t id)
{
    if (messageConstructors.find(id) == messageConstructors.end())
    {
        return NULL;
    }

    return messageConstructors[id]();
}
