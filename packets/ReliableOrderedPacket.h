#pragma once
#include <cstdint>
#include <type_traits>
#include <typeinfo>

#include "Packet.h"

// class that holds the reliability layer of the protocol. The sequence number
// ack'd packet range.
class ReliableOrderedPacket : public Packet
{
  public:
    IDENTIFIER(ReliableOrderedPacket, 'RLPK')
    SERIALIZER
    // ReliableOrderedPacket(int seq, uint8_t ackStart, uint8_t ackEnd)
    //     : sequenceNumber(seq), ackStart(ackStart), ackEnd(ackEnd){};
    // ReliableOrderedPacket(int seq) : sequenceNumber(seq), ackStart(seq), ackEnd(seq){};
    ReliableOrderedPacket(std::shared_ptr<MessageSerializer> factory) : Packet(factory)
    {
        messages = std::make_shared<std::vector<std::shared_ptr<Message>>>();
    }

    // TODO: Should this be a pointer to a vector? And just unique ptrs?
    std::shared_ptr<std::vector<std::shared_ptr<Message>>> messages;
    // Sequence number of this packet
    uint32_t sequenceNumber;
    //
    uint16_t ack;
    uint16_t numMessages;
    uint32_t ackBits;

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(sequenceNumber);
        stream.serialize(ack);
        stream.serialize(ackBits);
        // Write the number of messages
        SerializeMessages(stream);

        // check if reading. If so read messages use factory to read the
        // rest of the messages
        // if writing, use factory to write the rest
        return true;
    }

    bool SerializeMessages(InputMemoryBitStream& stream)
    {
        stream.serialize(numMessages);

        // read the message ids of the messages included in this packet
        uint16_t messageIds[32];
        for (int i = 0; i < numMessages; i++)
        {
            stream.serialize(messageIds[i]);
        }

        messages = messageFactory->ReadMessages(stream, numMessages);

        // go into the messages and assign their ids.Why aren't these in the messages
        // them selves? Well technically they aren't the ones that assign it
        // when they get created so it kinda makes sense.
        for (int i = 0; i < numMessages; i++)
        {
            (*messages)[i]->AssignId(messageIds[i]);
        }

        return true;
    }

    bool SerializeMessages(OutputMemoryBitStream& stream)
    {
        numMessages = messages->size();
        stream.serialize(numMessages);
        for (int i = 0; i < numMessages; i++)
        {
            stream.serialize((*messages)[i]->GetId());
        }

        messageFactory->WriteMessages(messages, stream);
        return true;
    }
};
