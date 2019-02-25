#include "PacketSerializer.h"
#include "InputMemoryBitStream.h"
#include "OutputMemoryBitStream.h"
#include "Packet.h"

bool PacketSerializer::AddConstructor(uint32_t id, PacketConstructor constructor)
{
    if (packetConstructors.find(id) != packetConstructors.end())
    {
        return false;
    }

    packetConstructors[id] = constructor;
    return true;
}

std::unique_ptr<Packet> PacketSerializer::CreatePacket(uint32_t id)
{
    if (packetConstructors.find(id) == packetConstructors.end())
    {
        return NULL;
    }

    return packetConstructors[id](messageFactory);
}

std::vector<std::unique_ptr<Packet>> PacketSerializer::ReadPackets(InputMemoryBitStream& in)
{
    uint32_t id;

    std::vector<std::unique_ptr<Packet>> packets;

    while (in.GetRemainingBitCount() > 0)
    {
        in.serialize(id);
        if (packetConstructors.find(id) == packetConstructors.end())
        {
            throw std::runtime_error("Bad Packet data!!!!");
        }

        std::unique_ptr<Packet> packet = packetConstructors[id](messageFactory);
        packet->Read(in);
        packets.push_back(std::move(packet));
    }

    return packets;
}

bool PacketSerializer::WritePackets(std::vector<std::unique_ptr<Packet>>& packets,
                                    OutputMemoryBitStream& out)
{
    for (auto const& packet : packets)
    {
        uint32_t id = packet->GetUniqueId();
        out.Write(id);
        packet->Write(out);
    }

    return true;
}

bool PacketSerializer::WritePacket(std::unique_ptr<Packet> packet, OutputMemoryBitStream& out)
{
    uint32_t id = packet->GetUniqueId();
    out.Write(id);
    packet->Write(out);

    return true;
}