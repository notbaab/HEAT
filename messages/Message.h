#pragma once

#include "InputMemoryBitStream.h"
#include "OutputMemoryBitStream.h"
#include <cstdint>
#include <functional>

#define SERIALIZER                                                                                 \
    bool Read(InputMemoryBitStream& stream) override { return Serialize(stream); }                 \
    bool Write(OutputMemoryBitStream& stream) override { return Serialize(stream); }

#define IDENTIFIER(cls, id)                                                                        \
    uint32_t GetUniqueId() const override { return cls::ID; }                                      \
    static const uint32_t ID = id;

// The same as a packet pretty much but holds game data. A distinction I'm
// not quite sure I want to make yet
class Message
{
  public:
    Message(){};
    virtual ~Message(){};

    virtual bool Read(InputMemoryBitStream& stream) = 0;
    virtual bool Write(OutputMemoryBitStream& stream) = 0;
    virtual uint32_t GetUniqueId() const = 0;
};
