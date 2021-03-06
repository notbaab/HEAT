#include "IO/InputMemoryBitStream.h"
#include "IO/OutputMemoryBitStream.h"
#include "catch.hpp"
#include "managers/PacketManager.h"
#include "messages/PlayerMessage.h"
using namespace Catch::literals;

// Round about way to see if the packet written contains all the expected
// messages
void checkMessageId(std::shared_ptr<Packet> packet, std::shared_ptr<PacketSerializer> packetSerializer,
                    int messageIdStart, int numMessages)
{
    const uint8_t* outPtr;
    uint32_t outSize;
    packetSerializer->WritePacket(packet, &outPtr, &outSize);
    int messageId = messageIdStart;

    auto copyOfData = std::make_unique<std::vector<uint8_t>>(outSize);
    memcpy(copyOfData->data(), outPtr, outSize);

    auto packets = packetSerializer->ReadPackets(std::move(copyOfData));

    for (int i = 0; i < packets.size(); ++i)
    {
        auto castRelPacket = static_cast<ReliableOrderedPacket*>(packets[i].get());
        REQUIRE(castRelPacket->messages->size() == numMessages);

        for (int i = 0; i < numMessages; ++i)
        {
            auto message = castRelPacket->messages->at(i);
            REQUIRE(message->GetId() == messageId);
            messageId++;
        }
    }
}

// helper that has the manager read a packet that contains only ack data
void ackPacket(PacketManager& manager, std::shared_ptr<PacketSerializer> packetSerializer, int ackNum, int ackBits,
               int sequenceNumber)
{
    auto ackedPacket = packetSerializer->CreatePacket(ReliableOrderedPacket::CLASS_ID);
    auto castPacket = std::static_pointer_cast<ReliableOrderedPacket>(ackedPacket);
    // say we have seen packet 0 by setting the ack and bit field appropriately
    castPacket->ack = ackNum;
    castPacket->ackBits = ackBits;

    // give it our own starting sequence number of 0
    castPacket->sequenceNumber = sequenceNumber;
    // no messages in this packet
    castPacket->numMessages = 0;
    manager.ReadPacket(castPacket);
}

void sendMessages(PacketManager& manager, std::shared_ptr<MessageSerializer> messageSerializer, uint8_t numMessages)
{
    // write out a packet with 5 messages
    for (int i = 0; i < numMessages; ++i)
    {
        auto msg = messageSerializer->CreateMessage(PlayerMessage::CLASS_ID);
        // auto msg = std::static_pointer_cast<PlayerMessage>(
        //     std::move());
        manager.SendMessage(std::move(msg));
    }
}

TEST_CASE("Packet Manager", "[manger]")
{

    auto messageSerializer = std::make_shared<MessageSerializer>();
    AddMessageCtor(messageSerializer, PlayerMessage);

    auto bitReader = std::make_unique<InputMemoryBitStream>();
    auto bitWriter = std::make_unique<OutputMemoryBitStream>();
    auto packetReader = std::make_unique<StructuredDataReader>(std::move(bitReader));
    auto packetWriter = std::make_unique<StructuredDataWriter>(std::move(bitWriter));

    auto packetSerializer =
        std::make_shared<PacketSerializer>(messageSerializer, std::move(packetReader), std::move(packetWriter));
    AddPacketCtor(packetSerializer, ReliableOrderedPacket);

    SECTION("Basic packet and message ack")
    {
        // Managers
        auto manager = PacketManager(packetSerializer);

        sendMessages(manager, messageSerializer, 5);
        auto packet = manager.WritePacket(ReliableOrderedPacket::CLASS_ID);
        checkMessageId(packet, packetSerializer, 0, 5);

        sendMessages(manager, messageSerializer, 3);
        packet = manager.WritePacket(ReliableOrderedPacket::CLASS_ID);
        checkMessageId(packet, packetSerializer, 5, 3);

        // advance just a little past our threshold of .1
        manager.SetTime(0.11);

        packet = manager.WritePacket(ReliableOrderedPacket::CLASS_ID);
        // Since we advanced time, the third packet will have all 8 messages since they
        // are unacked
        checkMessageId(packet, packetSerializer, 0, 8);

        // Start acking some packets
        // pretend to receive a packet from something that says we have received
        // the 0th packet
        ackPacket(manager, packetSerializer, 0, 1, 0);

        // move time ahead
        manager.SetTime(0.22);
        packet = manager.WritePacket(ReliableOrderedPacket::CLASS_ID);
        // fourth packet contains messages not included in the 0th packet
        checkMessageId(packet, packetSerializer, 5, 3);

        // ack second packet
        ackPacket(manager, packetSerializer, 1, 2, 1);

        // There shouldn't be any more messages to send
        packet = manager.WritePacket(ReliableOrderedPacket::CLASS_ID);
        // ack'd all packets, should have written no messages
        checkMessageId(packet, packetSerializer, 0, 0);
    }

    // SECTION("Client Server Test")
    // {
    //     auto client = PacketManager(packetSerializer);
    //     auto server = PacketManager(packetSerializer);

    //     sendMessages(client, messageSerializer, 5);

    //     auto packet = client.WritePacket(ReliableOrderedPacket::CLASS_ID);
    //     checkMessageId(packet, packetSerializer, 0, 5);

    //     // // read packet
    //     bool success = server.ReadPacket(packet);
    //     // REQUIRE(success);

    //     // // advance so it so server acks packet
    //     // server.SetTime(0.11);
    //     // packet = server.WritePacket(ReliableOrderedPacket::CLASS_ID);
    //     // success = client.ReadPacket(packet);

    //     // packet = client.WritePacket(ReliableOrderedPacket::CLASS_ID);

    //     // // next client packet won't send the olde messages
    //     // checkMessageId(packet, packetSerializer, 5, 0);
    //     // REQUIRE(success);
    // }
}