#pragma once
#include <cstdint>
#include <type_traits>
#include <typeinfo>

#include "Packet.h"
#include "logger/Logger.h"

// class that holds the reliability layer of the protocol. The sequence number
// ack'd packet range.
class ReliableOrderedPacket : public Packet
{
  public:
    CLASS_IDENTIFIER(ReliableOrderedPacket, 'RLPK')
    SERIALIZER

    ReliableOrderedPacket(std::shared_ptr<MessageSerializer> factory) : Packet(factory)
    {
        messages = std::make_shared<std::vector<std::shared_ptr<Message>>>();
    }

    // TODO: Should this be a pointer to a vector? And just unique ptrs?
    std::shared_ptr<std::vector<std::shared_ptr<Message>>> messages;

    // Sequence number of this packet
    uint32_t sequenceNumber;
    uint32_t ackBits;
    uint16_t ack;
    uint16_t numMessages;
    // Assigned externally by the packet manager when writing packets
    uint16_t messageIds[64];

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(sequenceNumber, "sequenceNumber");
        stream.serialize(ack, "ack");
        stream.serialize(ackBits, "ackBits");

        SerializeMessages(stream);

        // check if reading. If so read messages use factory to read the
        // rest of the messages
        // if writing, use factory to write the rest
        return true;
    }

  private:
    bool SerializeMessages(StructuredDataReader& stream)
    {
        stream.serialize(numMessages, "numMessages");
        if (numMessages > 64)
        {
            ERROR("FUCKING SHIT BALLS");
        }

        // THIS NEEDS A KEY
        stream.StartArray("messageIds");
        // stream.startArray(numMessages, "messages");
        // read the message ids of the messages included in this packet
        for (int i = 0; i < numMessages; i++)
        {
            stream.serialize(messageIds[i]);
        }

        stream.EndArray("messageIds");

        stream.StartArray("messages");
        messages = messageFactory->ReadMessages(stream, numMessages);
        stream.EndArray("messages");

        // go into the messages and assign their ids.
        // Why aren't these in the messages them selves you may ask? Well the
        // assignment happens by the packet manager to keep track of it
        for (int i = 0; i < numMessages; i++)
        {
            (*messages)[i]->AssignId(messageIds[i]);
        }

        return true;
    }

    bool SerializeMessages(StructuredDataWriter& stream)
    {
        numMessages = messages->size();
        stream.serialize(numMessages, "numMessages");

        stream.StartArray("messageIds");
        // PIMP There should be some sort of way to tell if the messageIds were
        // initialized. We are doing this based on trust now which makes me queasy
        for (int i = 0; i < numMessages; i++)
        {
            stream.serialize(messageIds[i]);
        }
        stream.EndArray();

        stream.StartArray("messages");
        messageFactory->WriteMessages(messages, stream);
        stream.EndArray();
        return true;
    }
};
