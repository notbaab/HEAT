#include "IO/InputMemoryBitStream.h"
#include "IO/OutputMemoryBitStream.h"
#include "catch.hpp"
#include "messages/MessageSerializer.h"
#include "messages/PlayerMessage.h"
#include "packets/PacketSerializer.h"
#include "packets/ReliableOrderedPacket.h"
#include <iostream>
#include <vector>
using namespace Catch::literals;

TEST_CASE("Packet Serialize Test", "[packet]")
{
    auto serializer = PacketSerializer();
    auto messageSerializer = MessageSerializer();
    AddPacketCtor(serializer, ReliableOrderedPacket);
    AddMessageCtor(messageSerializer, PlayerMessage);

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
        uint32_t id = ReliableOrderedPacket::ID;

        // This will fail if it can't read the id form the above packet
        auto firstPacket = static_cast<ReliableOrderedPacket*>(packets[0].get());
        auto secondPacket = static_cast<ReliableOrderedPacket*>(packets[1].get());

        REQUIRE(firstPacket->sequenceNumber == 1);
        REQUIRE(secondPacket->sequenceNumber == 0x1e233011);
    }

    SECTION("Write some reliable packet out")
    {
        auto firstPacket = std::make_unique<ReliableOrderedPacket>(1);
        auto secondPacket = std::make_unique<ReliableOrderedPacket>(0x1e233011);
        auto out = OutputMemoryBitStream();
        // std::vector<std::unique_ptr<Packet>> packet_vectors{std::move(firstPacket),
        //                                                     std::move(secondPacket)};
        std::vector<std::unique_ptr<Packet>> packet_vectors;
        packet_vectors.push_back(std::move(firstPacket));
        packet_vectors.push_back(std::move(secondPacket));
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

        auto fullPacketIn = std::make_unique<PlayerMessage>(
            PlayerMessage::ReplicationState::ALL_STATE, id, xVel, yVel, xLoc, yLoc);
        // Make one with only the id
        auto onlyIdIn = std::make_unique<PlayerMessage>(PlayerMessage::ReplicationState::PRS_PID,
                                                        id, xVel, yVel, xLoc, yLoc);
        // one with only the position
        auto onlyPositionIn = std::make_unique<PlayerMessage>(
            PlayerMessage::ReplicationState::PRS_POSI, id, xVel, yVel, xLoc, yLoc);

        auto out = OutputMemoryBitStream();
        std::vector<std::unique_ptr<Message>> message_vector;
        message_vector.push_back(std::move(fullPacketIn));
        message_vector.push_back(std::move(onlyIdIn));
        message_vector.push_back(std::move(onlyPositionIn));
        messageSerializer.WriteMessages(message_vector, out);

        // go from the raw byte array back to a packet
        auto rawChar = out.GetBufferPtr();
        auto in = InputMemoryBitStream(rawChar, out.GetBitLength());
        auto packets = messageSerializer.ReadMessages(in);
        auto fullPacketOut = static_cast<PlayerMessage*>(packets[0].get());
        auto onlyIdOut = static_cast<PlayerMessage*>(packets[1].get());
        auto onlyPositionOut = static_cast<PlayerMessage*>(packets[2].get());

        REQUIRE(fullPacketOut->id == 12);
        REQUIRE(fullPacketOut->xVel == 2.4f);
        REQUIRE(fullPacketOut->yVel == 2.9f);
        REQUIRE(fullPacketOut->xLoc == 2.2f);
        REQUIRE(fullPacketOut->yLoc == 2.0f);

        REQUIRE(onlyIdOut->id == 12);

        REQUIRE(onlyPositionOut->xVel == 2.4f);
        REQUIRE(onlyPositionOut->yVel == 2.9f);
        REQUIRE(onlyPositionOut->xLoc == 2.2f);
        REQUIRE(onlyPositionOut->yLoc == 2.0f);
    }
}
