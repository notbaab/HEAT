#pragma once

#include "InputMemoryBitStream.h"
#include "Message.h"
#include "MessageSerializer.h"
#include "OutputMemoryBitStream.h"
#include <cstdint>
#include <functional>

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
    virtual uint32_t GetIdentifier() const = 0;
};
