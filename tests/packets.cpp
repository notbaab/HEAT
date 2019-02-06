#include "IO/InputMemoryBitStream.h"
#include "IO/OutputMemoryBitStream.h"
#include "catch.hpp"
#include "packets/PacketSerializer.h"
#include "packets/PlayerPacket.h"
#include "packets/ReliabilityPacket.h"
#include <iostream>
#include <vector>
using namespace Catch::literals;

TEST_CASE("Packet Serialize Test", "[packet]")
{
    auto serializer = PacketSerializer();
    AddPacketCtor(serializer, ReliabilityPacket);
    AddPacketCtor(serializer, PlayerPacket);

    std::vector<uint8_t> firstRealPacket{'K', 'P', 'L', 'R', 0x01, 0x00, 0x00, 0x00};
    std::vector<uint8_t> secondRealPacket{'K', 'P', 'L', 'R', 0x11, 0x30, 0x23, 0x1e};
    std::vector<uint8_t> fullPackets(firstRealPacket.begin(), firstRealPacket.end());
    fullPackets.insert(fullPackets.end(), secondRealPacket.begin(), secondRealPacket.end());

    SECTION("Read a raw char vector to a packet")
    {
        auto packetPtr = std::make_shared<std::vector<uint8_t>>(fullPackets);
        // auto packetPtr = std::make_shared<std::vector<uint8_t>>(firstRealPacket);
        auto in = InputMemoryBitStream(packetPtr, packetPtr->size() * 8);
        auto packets = serializer.ReadPackets(in);
        uint32_t id = ReliabilityPacket::ID;

        // This will fail if it can't read the id form the above packet
        auto firstPacket = std::static_pointer_cast<ReliabilityPacket>(packets[0]);
        auto secondPacket = std::static_pointer_cast<ReliabilityPacket>(packets[1]);

        REQUIRE(firstPacket->sequenceNumber == 1);
        REQUIRE(secondPacket->sequenceNumber == 0x1e233011);
    }

    SECTION("Write some reliable packet out")
    {
        auto firstPacket = std::make_shared<ReliabilityPacket>(1);
        auto secondPacket = std::make_shared<ReliabilityPacket>(0x1e233011);
        auto out = OutputMemoryBitStream();
        std::vector<std::shared_ptr<Packet>> packet_vectors{firstPacket, secondPacket};
        serializer.WritePackets(packet_vectors, out);

        auto ptr = out.GetBufferPtr();
        for (int i = 0; i < out.GetByteLength(); ++i)
        {
            REQUIRE((*ptr)[i] == fullPackets[i]);
        }
    }

    SECTION("PlayerPacket")
    {
        uint32_t id = 12;
        float xVel = 2.4f;
        float yVel = 2.9f;
        float xLoc = 2.2f;
        float yLoc = 2.0f;

        auto fullPacketIn = std::make_shared<PlayerPacket>(
            PlayerPacket::ReplicationState::ALL_STATE, id, xVel, yVel, xLoc, yLoc);
        // Make one with only the id
        auto onlyIdIn = std::make_shared<PlayerPacket>(PlayerPacket::ReplicationState::PRS_PID, id,
                                                       xVel, yVel, xLoc, yLoc);
        // one with only the position
        auto onlyPositionIn = std::make_shared<PlayerPacket>(
            PlayerPacket::ReplicationState::PRS_POSI, id, xVel, yVel, xLoc, yLoc);

        auto out = OutputMemoryBitStream();
        std::vector<std::shared_ptr<Packet>> packet_vectors{fullPacketIn, onlyIdIn, onlyPositionIn};
        serializer.WritePackets(packet_vectors, out);

        // go from the raw byte array back to a packet
        auto rawChar = out.GetBufferPtr();
        auto in = InputMemoryBitStream(rawChar, out.GetBitLength());
        auto packets = serializer.ReadPackets(in);
        auto fullPacketOut = std::static_pointer_cast<PlayerPacket>(packets[0]);
        auto onlyIdOut = std::static_pointer_cast<PlayerPacket>(packets[1]);
        auto onlyPositionOut = std::static_pointer_cast<PlayerPacket>(packets[2]);

        REQUIRE(fullPacketOut->id == 12);
        REQUIRE(fullPacketOut->xVel == 2.4f);
        REQUIRE(fullPacketOut->yVel == 2.9f);
        REQUIRE(fullPacketOut->xLoc == 2.2f);
        REQUIRE(fullPacketOut->yLoc == 2.0f);

        REQUIRE(onlyIdOut->id == 12);

        REQUIRE(onlyPositionOut->id == 0);
        REQUIRE(onlyPositionOut->xVel == 2.4f);
        REQUIRE(onlyPositionOut->yVel == 2.9f);
        REQUIRE(onlyPositionOut->xLoc == 2.2f);
        REQUIRE(onlyPositionOut->yLoc == 2.0f);
    }
}
