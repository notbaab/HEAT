#pragma once

#include "IO/InputMemoryBitStream.h"
#include "IO/OutputMemoryBitStream.h"
#include <cstdint>
#include <functional>

#define SERIALIZER                                                                                 \
    bool Read(InputMemoryBitStream& stream) override { return Serialize(stream); }                 \
    bool Write(OutputMemoryBitStream& stream) override { return Serialize(stream); }

#define CLASS_IDENTIFIER(cls, class_id)                                                            \
    uint32_t GetClassIdentifier() const override { return cls::CLASS_ID; }                         \
    static const uint32_t CLASS_ID = class_id;

// The same as a packet pretty much but holds game data. A distinction I'm
// not quite sure I want to make yet
class Message
{
  public:
    Message() {}
    virtual ~Message() {}

    virtual bool Read(InputMemoryBitStream& stream) = 0;
    virtual bool Write(OutputMemoryBitStream& stream) = 0;
    virtual uint32_t GetClassIdentifier() const = 0;

    void AssignId(uint16_t id) { this->id = id; }
    uint16_t GetId() { return id; }

    //
    static std::string StringFromId(uint32_t identifier)
    {
        // Need to check endianness of the platform we are on.
        // Probably
        char buf[5] = "";
        buf[3] = identifier;
        buf[2] = identifier >> 8;
        buf[1] = identifier >> 16;
        buf[0] = identifier >> 24;
        return std::string(buf);
    }

    std::string IdentifierToString()
    {
        // May need to check endianness of the platform we are on.
        // Probably
        uint32_t identifier = GetClassIdentifier();
        auto s = StringFromId(identifier);
        return s;
    }

  protected:
    // essentially the sequence number
    uint16_t id;
};
