
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

class TestPacket : Packet
{
  public:
    TestPacket();
    ~TestPacket();
};

TEST_CASE("Packet Manager", "[manger]")
{
    // setup our serializers
    auto messageSerializer = std::make_shared<MessageSerializer>();
    AddMessageCtor(messageSerializer, PlayerMessage);
    auto packetSerializer = PacketSerializer(messageSerializer);
    AddPacketCtor(packetSerializer, ReliableOrderedPacket);

    // Managers
    auto clientManager = PacketManager(packetSerializer);
    auto serverManager = PacketManager(packetSerializer);

    // Stream data stuffs
    // auto packetPtr = std::make_shared<std::vector<uint8_t>>(fullPackets);

    // write out a packet with 5 messages
    for (int i = 0; i < 5; ++i)
    {
        auto msg = std::static_pointer_cast<PlayerMessage>(
            std::move(messageSerializer->CreateMessage(PlayerMessage::ID)));
        clientManager.SendMessage(msg);
    }
    auto firstPacket = clientManager.WritePacket();

    // write second packet with 3 messages
    for (int i = 0; i < 3; ++i)
    {
        auto msg = std::static_pointer_cast<PlayerMessage>(
            std::move(messageSerializer->CreateMessage(PlayerMessage::ID)));
        clientManager.SendMessage(msg);
        /* code */
    }

    auto secondPacket = clientManager.WritePacket();

    auto out = OutputMemoryBitStream();
    // check the first packet is good
    packetSerializer.WritePacket(firstPacket, out);

    auto rawChar = out.GetBufferPtr();
    auto in = InputMemoryBitStream(rawChar, out.GetBitLength());
    auto packets = packetSerializer.ReadPackets(in);

    int messageId = 0;

    // basic did it write out the 5 messages
    for (int i = 0; i < packets.size(); ++i)
    {
        auto castRelPacket = static_cast<ReliableOrderedPacket*>(packets[i].get());
        REQUIRE(castRelPacket->messages->size() == 5);
        for (auto msg : *castRelPacket->messages)
        {
            std::cout << msg->GetId() << std::endl;
            REQUIRE(msg->GetId() == messageId);
            messageId++;
        }
    }

    REQUIRE(messageId == 5);

    // check the second packet
    out = OutputMemoryBitStream();
    packetSerializer.WritePacket(secondPacket, out);

    rawChar = out.GetBufferPtr();
    in = InputMemoryBitStream(rawChar, out.GetBitLength());
    packets = packetSerializer.ReadPackets(in);

    // writes out the next 3 messages
    for (int i = 0; i < packets.size(); ++i)
    {
        auto castRelPacket = static_cast<ReliableOrderedPacket*>(packets[i].get());
        REQUIRE(castRelPacket->messages->size() == 3);
        for (auto msg : *castRelPacket->messages)
        {
            REQUIRE(msg->GetId() == messageId);
            messageId++;
        }
    }

    messageId = 0;
    // advance just a little past our threshold of .1
    clientManager.AdvanceTime(0.11);

    auto thirdPacket = clientManager.WritePacket();

    out = OutputMemoryBitStream();
    // check the first packet is good
    packetSerializer.WritePacket(thirdPacket, out);

    rawChar = out.GetBufferPtr();
    in = InputMemoryBitStream(rawChar, out.GetBitLength());
    packets = packetSerializer.ReadPackets(in);

    // should write all un ack'd messages
    for (int i = 0; i < packets.size(); ++i)
    {
        auto castRelPacket = static_cast<ReliableOrderedPacket*>(packets[i].get());
        REQUIRE(castRelPacket->messages->size() == 8);
        for (auto msg : *castRelPacket->messages)
        {
            REQUIRE(msg->GetId() == messageId);
            messageId++;
        }
    }

    // Start acking some messages

    // if we advance time, the next packet will include all the packets
    // that aren't ack'd, which should be all of them at this point

    // // next packe twill write the
    // packet = clientManager.WritePacket();

    // packetSerializer.WritePacket(packet, out);

    // rawChar = out.GetBufferPtr();
    // in = InputMemoryBitStream(rawChar, out.GetBitLength());
    // packets = packetSerializer.ReadPackets(in);

    // // basic did it put the messages into the thing
    // for (int i = 0; i < packets.size(); ++i)
    // {
    //     auto castRelPacket = static_cast<ReliableOrderedPacket*>(packets[i].get());
    //     for (auto msg : *castRelPacket->messages)
    //     {
    //         REQUIRE(msg->GetId() == messageId);
    //         messageId++;
    //     }
    // }
}