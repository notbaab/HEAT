#pragma once

#include "packets/Message.h"

#define EVENT_IDENTIFIER(cls, eventType)                                                           \
    uint32_t GetEventType() const override { return cls::EVENT_TYPE; }                             \
    static const uint32_t EVENT_TYPE = eventType;

// base class for an event to differentiate itself from the message class
// as of now it isn't actually different from the message so it might be
// unnecessary.
class Event : public Message
{
  public:
    virtual uint32_t GetEventType() const = 0;
    // override the identifier function to
    TYPE_IDENTIFIER(Event, 'EVTM');
};
