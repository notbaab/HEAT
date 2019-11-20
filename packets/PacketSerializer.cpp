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

std::shared_ptr<Packet> PacketSerializer::CreatePacket(uint32_t id)
{
    if (packetConstructors.find(id) == packetConstructors.end())
    {
        return NULL;
    }

    return packetConstructors[id](messageFactory);
}

std::vector<std::shared_ptr<Packet>>
PacketSerializer::ReadPackets(std::unique_ptr<std::vector<uint8_t>> data)
{
    auto in = InputMemoryBitStream(std::move(data));
    return ReadPackets(in);
}

std::vector<std::shared_ptr<Packet>> PacketSerializer::ReadPackets(InputMemoryBitStream& in)
{
    uint32_t id;

    std::vector<std::shared_ptr<Packet>> packets;

    while (in.GetRemainingBitCount() > 0)
    {
        in.serialize(id);
        if (packetConstructors.find(id) == packetConstructors.end())
        {
            std::cout << "bad data for id " << id << std::endl;
            throw std::runtime_error("Bad Packet data!!!!");
        }

        std::shared_ptr<Packet> packet = packetConstructors[id](messageFactory);
        packet->Read(in);
        packets.push_back(std::move(packet));
    }

    return packets;
}

bool PacketSerializer::WritePackets(std::vector<std::shared_ptr<Packet>>& packets,
                                    OutputMemoryBitStream& out)
{
    for (auto const& packet : packets)
    {
        WritePacket(packet, out);
    }

    return true;
}

bool PacketSerializer::WritePacket(std::shared_ptr<Packet> packet, OutputMemoryBitStream& out)
{
    uint32_t id = packet->GetIdentifier();
    out.Write(id);
    packet->Write(out);

    return true;
}