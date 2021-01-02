#pragma once
#include <memory>

class MessageSerializer;
class PacketSerializer;

namespace holistic
{
void SetupMessageSerializer(std::shared_ptr<MessageSerializer> serializer);
void SetupPacketSerializer(std::shared_ptr<PacketSerializer> serializer);
} // namespace holistic