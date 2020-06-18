#pragma once

#include <assert.h>
#include <cstdint>
#include <functional>

#include "IO/InputMemoryBitStream.h"
#include "IO/OutputMemoryBitStream.h"

#define SERIALIZER                                                                                                     \
    bool Read(InputMemoryBitStream& stream) override { return Serialize(stream); }                                     \
    bool Write(OutputMemoryBitStream& stream) override { return Serialize(stream); }

#define CLASS_IDENTIFIER(cls, class_id)                                                                                \
    uint32_t GetClassIdentifier() const override { return cls::CLASS_ID; }                                             \
    static const uint32_t CLASS_ID = class_id;

#define TYPE_IDENTIFIER(cls, typeId)                                                                                   \
    uint32_t GetTypeIdentifier() const override { return cls::TYPE_ID; }                                               \
    static const uint32_t TYPE_ID = typeId;

const uint32_t MESSAGE_TYPE_ID = 'BMSG';

// The same as a packet pretty much but holds game data. A distinction I'm
// not quite sure I want to make yet
class Message
{
  public:
    Message() { this->idAssigned = false; }
    virtual ~Message() {}

    virtual bool Read(InputMemoryBitStream& stream) = 0;
    virtual bool Write(OutputMemoryBitStream& stream) = 0;
    virtual uint32_t GetClassIdentifier() const = 0;

    // Can't use the #define cause it doesn't override anything since it's the base class
    virtual uint32_t GetTypeIdentifier() const { return MESSAGE_TYPE_ID; }

    // Messages are assigned IDs when they are read only. When writing
    // we write the id in a different way...
    void AssignId(uint16_t id)
    {
        this->idAssigned = true;
        this->id = id;
    }

    uint16_t GetId()
    {
        assert(idAssigned);
        return id;
    }

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
    bool idAssigned;
};
