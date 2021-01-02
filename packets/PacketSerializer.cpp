#include "PacketSerializer.h"
#include "IO/StructuredDataReader.h"
#include "IO/StructuredDataWriter.h"
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

std::vector<std::shared_ptr<Packet>> PacketSerializer::ReadPackets(std::unique_ptr<std::vector<uint8_t>> data)
{
    uint32_t id;
    std::vector<std::shared_ptr<Packet>> packets;

    reader->SetStreamBuffer(std::move(data));
    while (reader->HasMoreData())
    {
        reader->StartObject();

        reader->serialize(id, "id");

        if (packetConstructors.find(id) == packetConstructors.end())
        {
            std::cout << "bad data for id " << id << std::endl;
            throw std::runtime_error("Bad Packet data!!!!");
        }

        std::shared_ptr<Packet> packet = packetConstructors[id](messageFactory);
        packet->Read(*reader);
        packets.push_back(std::move(packet));

        reader->EndObject();
    }

    return packets;
}

// Give back a pointer to the underlying buffer where the packet was written to
bool PacketSerializer::WritePacket(std::shared_ptr<Packet> packet, const uint8_t** outBuff, uint32_t* outSize)
{
    // Clear the writer output buffer, we're filling it from the top
    writer->ResetBuffer();
    uint32_t id = packet->GetClassIdentifier();

    writer->StartObject();
    writer->serialize(id, "id");
    packet->Write(*writer);
    writer->EndObject();

    *outBuff = writer->GetRawBuffer();
    *outSize = writer->GetByteLength();

    return true;
}
