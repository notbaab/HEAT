#include "IO/JsonInputMemoryStream.h"
#include "IO/JsonOutputMemoryStream.h"
#include "catch.hpp"
#include "holistic/SetupSerializers.h"
#include "messages/PlayerMessage.h"
#include "packets/MessageSerializer.h"
#include "packets/PacketSerializer.h"
#include "packets/ReliableOrderedPacket.h"
#include "packets/UnauthenticatedPacket.h"

using namespace Catch::literals;

TEST_CASE("Packet Json Serialize Test", "[packet]")
{
    auto reader = std::make_unique<JsonInputMemoryStream>();
    auto writer = std::make_unique<JsonOutputMemoryStream>();
    auto packetReader = std::make_unique<StructuredDataReader>(std::move(reader));
    auto packetWriter = std::make_unique<StructuredDataWriter>(std::move(writer));

    auto messageSerializer = std::make_shared<MessageSerializer>();
    auto packetSerializer =
        std::make_shared<PacketSerializer>(messageSerializer, std::move(packetReader), std::move(packetWriter));

    holistic::SetupMessageSerializer(messageSerializer);
    holistic::SetupPacketSerializer(packetSerializer);

    const char* raw_json_data =
        R"({"id":1380733003,"sequenceNumber":2,"ack":10,"ackBits":42,"numMessages":2,"messageIds":[32,33],"messages":[{"id":1129336655,"playerId":12,"objType":0,"worldId":0},{"id":810568005,"worldId":1234,"xDelta":1,"yDelta":4,"moveSeq":32}]})";

    auto jsonByteArray = std::make_shared<std::vector<uint8_t>>();
    for (int i = 0; i < strlen(raw_json_data); ++i)
    {
        jsonByteArray->push_back(raw_json_data[i]);
    }

    SECTION("Read a raw char vector to a packet")
    {
        auto jsonReader = std::make_unique<JsonInputMemoryStream>();
        auto jsonWriter = std::make_unique<JsonOutputMemoryStream>();

        jsonReader->SetInputBuffer(jsonByteArray);

        uint32_t id, playerId;

        // read some data
        jsonReader->StartObject();
        jsonReader->Read(id, "id");
        REQUIRE(id == 1380733003);

        jsonReader->StartArray("messages");
        jsonReader->StartObject();

        jsonReader->Read(id, "id");
        jsonReader->Read(playerId, "playerId");

        REQUIRE(id == 1129336655);
        REQUIRE(playerId == 12);

        jsonReader->EndObject();
        jsonReader->StartObject();

        uint32_t worldId;
        jsonReader->Read(id, "id");
        jsonReader->Read(worldId, "worldId");
        REQUIRE(id == 810568005);
        REQUIRE(worldId == 1234);
        jsonReader->EndObject();
        jsonReader->EndArray();

        // Should be back in the outer scope
        uint32_t numMessages;
        jsonReader->Read(numMessages, "numMessages");
        jsonReader->StartArray("messageIds");
        uint32_t messageId;
        for (int i = 0; i < numMessages; ++i)
        {
            jsonReader->Read(messageId, "");
        }
        jsonReader->EndArray();

        auto packetPtr = std::make_unique<std::vector<uint8_t>>();
        for (int i = 0; i < jsonByteArray->size(); i++)
        {
            packetPtr->push_back(jsonByteArray->at(i));
        }

        auto packets = packetSerializer->ReadPackets(std::move(packetPtr));
        id = ReliableOrderedPacket::CLASS_ID;

        // This will fail if it can't read the id form the above packet
        auto firstPacket = static_cast<ReliableOrderedPacket*>(packets[0].get());

        // Make sure it wrote all the stuff out
        REQUIRE(firstPacket->sequenceNumber == 2);

        REQUIRE(firstPacket->ack == 10);

        REQUIRE(firstPacket->ackBits == 42);

        REQUIRE(firstPacket->messages->size() == 2);
    }

    SECTION("Write some reliable packet out")
    {
        //     auto firstPacket = packetSerializer->CreatePacket(ReliableOrderedPacket::CLASS_ID);
        //     auto castFirstPacket = static_cast<ReliableOrderedPacket*>(firstPacket.get());
        //     castFirstPacket->sequenceNumber = 1;
        //     castFirstPacket->ack = 1;
        //     castFirstPacket->ackBits = 0x10500401;
        //     auto secondPacket = packetSerializer->CreatePacket(ReliableOrderedPacket::CLASS_ID);
        //     auto castSecondPacket = static_cast<ReliableOrderedPacket*>(secondPacket.get());
        //     castSecondPacket->sequenceNumber = 0x1e233011;
        //     castSecondPacket->ack = 0x3001;
        //     castSecondPacket->ackBits = 0x10501401;

        //     auto out = OutputMemoryBitStream();

        //     const uint8_t* outPtr;
        //     uint32_t outSize;
        //     packetSerializer->WritePacket(firstPacket, &outPtr, &outSize);

        //     for (int i = 0; i < outSize; ++i)
        //     {
        //         REQUIRE(outPtr[i] == fullPackets[i]);
        //     }

        //     uint32_t shiftAmount = outSize;
        //     packetSerializer->WritePacket(secondPacket, &outPtr, &outSize);

        //     for (int i = 0; i < outSize; ++i)
        //     {
        //         REQUIRE(outPtr[i] == fullPackets[i + shiftAmount]);
        //     }
    }

    // TODO: Need to redue this one
    // SECTION("Raw Data Read of Unauthenticated Packet")
    // {
    //     std::vector<uint8_t> packet{'K',  'P',  'R',  'U',  0x01, 0x32, 0x21,
    //                                 0x01, 0x01, 0x00, 0x00, 0x00, 0x00};

    //     std::vector<uint8_t> padding;
    //     padding.reserve(1000);
    //     padding.assign(1000, 0);

    //     packet.insert(std::end(packet), std::begin(padding), std::end(padding));

    //     auto packetPtr = std::make_shared<std::vector<uint8_t>>(packet);
    //     auto in = InputMemoryBitStream(packetPtr);
    //     auto packets = packetSerializer->ReadPackets(in);
    //     uint32_t id = UnauthenticatedPacket::CLASS_ID;

    //     // This will fail if it can't read the id form the above packet
    //     auto firstPacket = static_cast<UnauthenticatedPacket*>(packets[0].get());

    //     REQUIRE(firstPacket->sequenceNumber == 1);

    //     auto basePacket = packetSerializer->CreatePacket(UnauthenticatedPacket::CLASS_ID);
    //     auto castPacket = std::static_pointer_cast<UnauthenticatedPacket>(basePacket);
    //     castPacket->sequenceNumber = 1;

    //     auto out = OutputMemoryBitStream();

    //     std::vector<std::shared_ptr<Packet>> packet_vectors;
    //     packet_vectors.push_back(std::move(castPacket));
    //     packetSerializer->WritePackets(packet_vectors, out);

    //     auto ptr = out.GetBufferPtr();
    //     for (int i = 0; i < out.GetByteLength(); ++i)
    //     {
    //         REQUIRE((*ptr)[i] == packet[i]);
    //     }
    // }

    // SECTION("PlayerMessage")
    // {
    //     float xVel = 2.4f;
    //     float yVel = 2.9f;
    //     float xLoc = 2.2f;
    //     float yLoc = 2.0f;

    //     auto fullPacketIn =
    //         std::make_unique<PlayerMessage>(PlayerMessage::ReplicationState::ALL_STATE, xVel, yVel, xLoc, yLoc);
    //     fullPacketIn->AssignId(14);
    //     // Make one with only the id
    //     auto onlyIdIn =
    //         std::make_unique<PlayerMessage>(PlayerMessage::ReplicationState::PRS_PID, xVel, yVel, xLoc, yLoc);
    //     onlyIdIn->AssignId(12);
    //     // one with only the position
    //     auto onlyPositionIn =
    //         std::make_unique<PlayerMessage>(PlayerMessage::ReplicationState::PRS_POSI, xVel, yVel, xLoc, yLoc);

    //     auto out = OutputMemoryBitStream();
    //     auto message_vector = std::make_unique<std::vector<std::shared_ptr<Message>>>();
    //     message_vector->push_back(std::move(fullPacketIn));
    //     message_vector->push_back(std::move(onlyIdIn));
    //     message_vector->push_back(std::move(onlyPositionIn));

    //     auto packet = packetSerializer->CreatePacket(ReliableOrderedPacket::CLASS_ID);
    //     auto castPacket = static_cast<ReliableOrderedPacket*>(packet.get());
    //     castPacket->messages = std::move(message_vector);

    //     castPacket->messageIds[0] = 14;
    //     castPacket->messageIds[1] = 12;

    //     packetSerializer->WritePacket(std::move(packet), out);

    //     // go from the raw byte array back to a packet
    //     auto rawChar = out.GetBufferPtr();
    //     auto in = InputMemoryBitStream(rawChar, out.GetBitLength());
    //     auto packets = packetSerializer->ReadPackets(in);
    //     castPacket = static_cast<ReliableOrderedPacket*>(packets[0].get());
    //     auto messages = castPacket->messages;

    //     auto fullPacketOut = static_cast<PlayerMessage*>((*messages)[0].get());
    //     auto onlyIdOut = static_cast<PlayerMessage*>((*messages)[1].get());
    //     auto onlyPositionOut = static_cast<PlayerMessage*>((*messages)[2].get());

    //     REQUIRE(fullPacketOut->GetId() == 14);
    //     REQUIRE(fullPacketOut->xVel == 2.4f);
    //     REQUIRE(fullPacketOut->yVel == 2.9f);
    //     REQUIRE(fullPacketOut->xLoc == 2.2f);
    //     REQUIRE(fullPacketOut->yLoc == 2.0f);

    //     REQUIRE(onlyIdOut->GetId() == 12);

    //     REQUIRE(onlyPositionOut->xVel == 2.4f);
    //     REQUIRE(onlyPositionOut->yVel == 2.9f);
    //     REQUIRE(onlyPositionOut->xLoc == 2.2f);
    //     REQUIRE(onlyPositionOut->yLoc == 2.0f);
    // }
}
