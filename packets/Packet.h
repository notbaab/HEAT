#pragma once

#include "InputMemoryBitStream.h"
#include "Message.h"
#include "MessageSerializer.h"
#include "OutputMemoryBitStream.h"
#include <cstdint>
#include <functional>

#define SERIALIZER                                                                                 \
    bool Read(InputMemoryBitStream& stream) override { return Serialize(stream); }                 \
    bool Write(OutputMemoryBitStream& stream) override { return Serialize(stream); }

#define IDENTIFIER(cls, id)                                                                        \
    uint32_t GetUniqueId() const override { return cls::ID; }                                      \
    static const uint32_t ID = id;

class Packet
{
  public:
    Packet(std::shared_ptr<MessageSerializer> factory) : messageFactory(factory)
    {
        auto x = messageFactory->CreateMessage('PLAY');
    };
    std::shared_ptr<MessageSerializer> messageFactory;
    virtual ~Packet(){};

    virtual bool Read(InputMemoryBitStream& stream) = 0;
    virtual bool Write(OutputMemoryBitStream& stream) = 0;
    virtual uint32_t GetUniqueId() const = 0;
};
