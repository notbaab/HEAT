#pragma once
#include "Packet.h"
#include <cstdint>
#include <type_traits>
#include <typeinfo>

// class that holds the reliability layer of the protocol. The sequence number
// ack'd packet range.
class ReliabilityPacket : public Packet
{
  public:
    IDENTIFIER(ReliabilityPacket, 'RLPK');
    SERIALIZER;
    ReliabilityPacket(int seq) : sequenceNumber(seq){};
    ReliabilityPacket(){};
    int sequenceNumber;

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(sequenceNumber);
        return true;
    }

  private:
};
