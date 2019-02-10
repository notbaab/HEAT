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
    ReliableOrderedPacket(int seq, uint8_t ackStart, uint8_t ackEnd)
        : sequenceNumber(seq), ackStart(ackStart), ackEnd(ackEnd){};
    ReliableOrderedPacket(int seq) : sequenceNumber(seq), ackStart(seq), ackEnd(seq){};
    ReliableOrderedPacket(){};
    // Sequence number of this packet
    int sequenceNumber;
    //
    uint8_t ackStart, ackEnd;

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(sequenceNumber);
        return true;
    }

  private:
};
