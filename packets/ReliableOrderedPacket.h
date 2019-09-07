#pragma once
#include "Packet.h"
#include <cstdint>
#include <type_traits>
#include <typeinfo>

// class that holds the reliability layer of the protocol. The sequence number
// ack'd packet range.
class ReliableOrderedPacket : public Packet
{
  public:
    IDENTIFIER(ReliableOrderedPacket, 'RLPK');
    SERIALIZER;
    // ReliableOrderedPacket(int seq, uint8_t ackStart, uint8_t ackEnd)
    //     : sequenceNumber(seq), ackStart(ackStart), ackEnd(ackEnd){};
    // ReliableOrderedPacket(int seq) : sequenceNumber(seq), ackStart(seq), ackEnd(seq){};
    ReliableOrderedPacket(std::shared_ptr<MessageSerializer> factory) : Packet(factory)
    {
        messages = std::make_shared<std::vector<std::shared_ptr<Message>>>();
    };

    // TODO: Should this be a pointer to a vector? And just unique ptrs?
    std::shared_ptr<std::vector<std::shared_ptr<Message>>> messages;
    // Sequence number of this packet
    int sequenceNumber;
    //
    uint16_t ack;
    uint8_t numMessages;
    uint32_t ackBits;

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(sequenceNumber);
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
        messages = std::move(messageFactory->ReadMessages(stream, numMessages));
        // if writing, use factory to write the rest
        return true;
    }

    bool SerializeMessages(OutputMemoryBitStream& stream)
    {
        numMessages = messages->size();
        stream.serialize(numMessages);
        messageFactory->WriteMessages(messages, stream);

        return true;
    }

  private:
};
