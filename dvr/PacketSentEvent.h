#pragma once

#include <memory>

#include "events/Event.h"
#include "networking/PacketInfo.h"

class PacketSentEvent : public Event
{
  public:
    CLASS_IDENTIFIER(PacketSentEvent, 'PKSE')
    EVENT_IDENTIFIER(PacketSentEvent, 'PKSE')

    bool Read(StructuredDataReader& reader) override { return packet.packet->Read(reader); }
    bool Write(StructuredDataWriter& stream) override { return packet.packet->Write(stream); }

    PacketSentEvent(){};

    // PIMP: Make a pointer? I'm not sure, the actual packet is a pointer and
    // that's technically the heavier object
    PacketInfo packet;
};

