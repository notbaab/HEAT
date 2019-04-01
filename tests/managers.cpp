
#include "IO/InputMemoryBitStream.h"
#include "IO/OutputMemoryBitStream.h"
#include "catch.hpp"
#include "managers/PacketManager.h"
#include "messages/PlayerMessage.h"
#include "packets/MessageSerializer.h"
#include "packets/Packet.h"
#include "packets/PacketSerializer.h"
#include "packets/ReliableOrderedPacket.h"
#include <iostream>
#include <vector>
using namespace Catch::literals;

// Round about way to see if the packet written contains all the expected
// messages
void checkMessageId(std::shared_ptr<Packet> packet,
                    std::shared_ptr<PacketSerializer> packetSerializer, int messageIdStart,
                    int numMessages)
{
    auto out = OutputMemoryBitStream();
    packetSerializer->WritePacket(packet, out);
    int messageId = messageIdStart;

    auto rawChar = out.GetBufferPtr();
    auto in = InputMemoryBitStream(rawChar, out.GetBitLength());
    auto packets = packetSerializer->ReadPackets(in);

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
void ackPacket(PacketManager& manager, std::shared_ptr<PacketSerializer> packetSerializer,
               int ackNum, int ackBits, int sequenceNumber)
{
    auto ackedPacket = packetSerializer->CreatePacket(ReliableOrderedPacket::ID);
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

void sendMessages(PacketManager& manager, std::shared_ptr<MessageSerializer> messageSerializer,
                  uint8_t numMessages)
{
    // write out a packet with 5 messages
    for (int i = 0; i < numMessages; ++i)
    {
        auto msg = std::static_pointer_cast<PlayerMessage>(
            std::move(messageSerializer->CreateMessage(PlayerMessage::ID)));
        manager.SendMessage(msg);
    }
}

TEST_CASE("Packet Manager", "[manger]")
{
    // setup our serializers
    auto messageSerializer = std::make_shared<MessageSerializer>();
    AddMessageCtor(messageSerializer, PlayerMessage);
    auto packetSerializer = std::make_shared<PacketSerializer>(messageSerializer);
    AddPacketCtor(packetSerializer, ReliableOrderedPacket);

    // Managers
    auto clientManager = PacketManager(packetSerializer);
    auto serverManager = PacketManager(packetSerializer);

    sendMessages(clientManager, *messageSerializer, 5);
    auto packet = clientManager.WritePacket();
    checkMessageId(packet, packetSerializer, 0, 5);

    sendMessages(clientManager, *messageSerializer, 3);
    packet = clientManager.WritePacket();
    checkMessageId(packet, packetSerializer, 5, 3);

    // advance just a little past our threshold of .1
    clientManager.SetTime(0.11);

    packet = clientManager.WritePacket();
    // Since we advanced time, the third packet will have all 8 messages since they
    // are unacked
    checkMessageId(packet, packetSerializer, 0, 8);

    // Start acking some packets
    // pretend to receive a packet from something that says we have received
    // the 0th packet
    ackPacket(clientManager, packetSerializer, 0, 1, 0);

    // move time ahead
    clientManager.SetTime(0.22);
    packet = clientManager.WritePacket();
    // fourth packet contains messages not included in the 0th packet
    checkMessageId(packet, packetSerializer, 5, 3);

    // ack second packet
    ackPacket(clientManager, packetSerializer, 1, 2, 1);

    // There shouldn't be any more messages to send
    packet = clientManager.WritePacket();
    // ack'd all packets, should have written no messages
    checkMessageId(packet, packetSerializer, 0, 0);
}