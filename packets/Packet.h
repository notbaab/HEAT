#pragma once

#include <cstdint>
#include <functional>

#include "Message.h"
#include "MessageSerializer.h"

class StructuredDataWriter;
class StructuredDataReader;

class Packet
{
  public:
    Packet(std::shared_ptr<MessageSerializer> factory) : messageFactory(factory) {}
    std::shared_ptr<MessageSerializer> messageFactory;
    virtual ~Packet() {}

    virtual bool Read(StructuredDataReader& stream) = 0;
    virtual bool Write(StructuredDataWriter& stream) = 0;
    virtual uint32_t GetClassIdentifier() const = 0;
};
