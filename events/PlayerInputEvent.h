#pragma once

#include <memory>

#include "Event.h"

// Event that says received control input
class PlayerInputEvent : public Event
{
  public:
    CLASS_IDENTIFIER(PlayerInputEvent, '0PIE')
    EVENT_IDENTIFIER(PlayerInputEvent, '0PIE')
    SERIALIZER

    PlayerInputEvent(){};
    PlayerInputEvent(uint8_t xDelta, uint8_t yDelta, uint32_t moveSeq)
        : xDelta(xDelta), yDelta(yDelta), moveSeq(moveSeq){};

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(xDelta);
        stream.serialize(yDelta);
        stream.serialize(moveSeq);
        return true;
    }

    // Move sequence number is used to be able to tell which moves the server
    // has seen. Technically with the reliability layer, we have all the info
    // we need to determine if we dig it out but it's a bit easier
    // if we control it at a high level.
    uint32_t moveSeq;
    int8_t xDelta, yDelta;
};
