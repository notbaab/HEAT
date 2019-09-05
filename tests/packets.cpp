#include "IO/InputMemoryBitStream.h"
#include "IO/OutputMemoryBitStream.h"
#include "catch.hpp"
#include "messages/PlayerMessage.h"
#include "packets/MessageSerializer.h"
#include "packets/PacketSerializer.h"
#include "packets/ReliableOrderedPacket.h"
#include "packets/UnauthenticatedPacket.h"
#include <iostream>
#include <vector>
using namespace Catch::literals;

TEST_CASE("Packet Serialize Test", "[packet]")
{
    auto messageSerializer = std::make_shared<MessageSerializer>();
    auto packetSerializer = std::make_shared<PacketSerializer>(messageSerializer);
    AddPacketCtor(packetSerializer, ReliableOrderedPacket);
    AddPacketCtor(packetSerializer, UnauthenticatedPacket);
    AddMessageCtor(messageSerializer, PlayerMessage);

    std::vector<uint8_t> firstRealPacket{'K', 'P', 'L', 'R', 0x01, 0x00, 0x00, 0x00, 0x00};
    std::vector<uint8_t> secondRealPacket{'K', 'P', 'L', 'R', 0x11, 0x30, 0x23, 0x1e, 0x00};
    std::vector<uint8_t> fullPackets(firstRealPacket.begin(), firstRealPacket.end());
    fullPackets.insert(fullPackets.end(), secondRealPacket.begin(), secondRealPacket.end());

    SECTION("Read a raw char vector to a packet")
    {
        auto packetPtr = std::make_shared<std::vector<uint8_t>>(fullPackets);
        auto in = InputMemoryBitStream(packetPtr);
        auto packets = packetSerializer->ReadPackets(in);
        uint32_t id = ReliableOrderedPacket::ID;

        // This will fail if it can't read the id form the above packet
        auto firstPacket = static_cast<ReliableOrderedPacket*>(packets[0].get());
        auto secondPacket = static_cast<ReliableOrderedPacket*>(packets[1].get());

        REQUIRE(firstPacket->sequenceNumber == 1);
        REQUIRE(secondPacket->sequenceNumber == 0x1e233011);
        REQUIRE(firstPacket->messages->size() == 0);
        REQUIRE(secondPacket->messages->size() == 0);
    }

    SECTION("Write some reliable packet out")
    {
        auto firstPacket = packetSerializer->CreatePacket(ReliableOrderedPacket::ID);
        auto castFirstPacket = static_cast<ReliableOrderedPacket*>(firstPacket.get());
        castFirstPacket->sequenceNumber = 1;
        auto secondPacket = packetSerializer->CreatePacket(ReliableOrderedPacket::ID);
        auto castSecondPacket = static_cast<ReliableOrderedPacket*>(secondPacket.get());
        castSecondPacket->sequenceNumber = 0x1e233011;

        auto out = OutputMemoryBitStream();

        std::vector<std::shared_ptr<Packet>> packet_vectors;
        packet_vectors.push_back(std::move(firstPacket));
        packet_vectors.push_back(std::move(secondPacket));
        packetSerializer->WritePackets(packet_vectors, out);

        auto ptr = out.GetBufferPtr();
        for (int i = 0; i < out.GetByteLength(); ++i)
        {
            REQUIRE((*ptr)[i] == fullPackets[i]);
        }
    }

    SECTION("Raw Data Read of Unauthenticated Packet")
    {
        std::vector<uint8_t> packet{'K',  'P',  'R',  'U',  0x01, 0x32, 0x21,
                                    0x01, 0x01, 0x00, 0x00, 0x00, 0x00};

        std::vector<uint8_t> padding;
        padding.reserve(1000);
        padding.assign(1000, 0);
        uint32_t expectedSalt = 0x01213201;

        packet.insert(std::end(packet), std::begin(padding), std::end(padding));

        auto packetPtr = std::make_shared<std::vector<uint8_t>>(packet);
        auto in = InputMemoryBitStream(packetPtr);
        auto packets = packetSerializer->ReadPackets(in);
        uint32_t id = UnauthenticatedPacket::ID;

        // This will fail if it can't read the id form the above packet
        auto firstPacket = static_cast<UnauthenticatedPacket*>(packets[0].get());

        REQUIRE(firstPacket->sequenceNumber == 1);
        REQUIRE(firstPacket->clientSalt == expectedSalt);

        auto basePacket = packetSerializer->CreatePacket(UnauthenticatedPacket::ID);
        auto castPacket = std::static_pointer_cast<UnauthenticatedPacket>(basePacket);
        castPacket->sequenceNumber = 1;
        castPacket->clientSalt = expectedSalt;

        auto out = OutputMemoryBitStream();

        std::vector<std::shared_ptr<Packet>> packet_vectors;
        packet_vectors.push_back(std::move(castPacket));
        packetSerializer->WritePackets(packet_vectors, out);

        auto ptr = out.GetBufferPtr();
        for (int i = 0; i < out.GetByteLength(); ++i)
        {
            REQUIRE((*ptr)[i] == packet[i]);
        }
    }

    SECTION("PlayerMessage")
    {
        float xVel = 2.4f;
        float yVel = 2.9f;
        float xLoc = 2.2f;
        float yLoc = 2.0f;

        auto fullPacketIn = std::make_unique<PlayerMessage>(
            PlayerMessage::ReplicationState::ALL_STATE, xVel, yVel, xLoc, yLoc);
        fullPacketIn->AssignId(14);
        // Make one with only the id
        auto onlyIdIn = std::make_unique<PlayerMessage>(PlayerMessage::ReplicationState::PRS_PID,
                                                        xVel, yVel, xLoc, yLoc);
        onlyIdIn->AssignId(12);
        // one with only the position
        auto onlyPositionIn = std::make_unique<PlayerMessage>(
            PlayerMessage::ReplicationState::PRS_POSI, xVel, yVel, xLoc, yLoc);

        auto out = OutputMemoryBitStream();
        auto message_vector = std::make_unique<std::vector<std::shared_ptr<Message>>>();
        message_vector->push_back(std::move(fullPacketIn));
        message_vector->push_back(std::move(onlyIdIn));
        message_vector->push_back(std::move(onlyPositionIn));

        auto packet = packetSerializer->CreatePacket(ReliableOrderedPacket::ID);
        auto castPacket = static_cast<ReliableOrderedPacket*>(packet.get());
        castPacket->messages = std::move(message_vector);

        packetSerializer->WritePacket(std::move(packet), out);

        // go from the raw byte array back to a packet
        auto rawChar = out.GetBufferPtr();
        auto in = InputMemoryBitStream(rawChar, out.GetBitLength());
        auto packets = packetSerializer->ReadPackets(in);
        castPacket = static_cast<ReliableOrderedPacket*>(packets[0].get());
        auto messages = castPacket->messages;

        auto fullPacketOut = static_cast<PlayerMessage*>((*messages)[0].get());
        auto onlyIdOut = static_cast<PlayerMessage*>((*messages)[1].get());
        auto onlyPositionOut = static_cast<PlayerMessage*>((*messages)[2].get());

        REQUIRE(fullPacketOut->GetId() == 14);
        REQUIRE(fullPacketOut->xVel == 2.4f);
        REQUIRE(fullPacketOut->yVel == 2.9f);
        REQUIRE(fullPacketOut->xLoc == 2.2f);
        REQUIRE(fullPacketOut->yLoc == 2.0f);

        REQUIRE(onlyIdOut->GetId() == 12);

        REQUIRE(onlyPositionOut->xVel == 2.4f);
        REQUIRE(onlyPositionOut->yVel == 2.9f);
        REQUIRE(onlyPositionOut->xLoc == 2.2f);
        REQUIRE(onlyPositionOut->yLoc == 2.0f);
    }
}
