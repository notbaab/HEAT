#pragma once

#include "InputMemoryBitStream.h"
#include "OutputMemoryBitStream.h"
#include <cstdint>
#include <functional>

// class Packet;

#define SERIALIZER                                                                                 \
    bool Read(InputMemoryBitStream& stream) override { return Serialize(stream); }                 \
    bool Write(OutputMemoryBitStream& stream) override { return Serialize(stream); }

#define IDENTIFIER(cls, id)                                                                        \
    uint32_t GetUniqueId() const override { return cls::ID; }                                      \
    static const uint32_t ID = id;

class Packet
{
  public:
    Packet(){};

    virtual bool Read(InputMemoryBitStream& stream) = 0;
    virtual bool Write(OutputMemoryBitStream& stream) = 0;
    virtual uint32_t GetUniqueId() const = 0;
};
