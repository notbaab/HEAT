#include <fstream>
#include <iostream>

#include "DVR.h"
#include "IO/InputMemoryBitStream.h"
#include "IO/OutputMemoryBitStream.h"
#include "IO/StructuredDataWriter.h"
#include "holistic/SetupSerializers.h"
#include "logger/Logger.h"
#include "packets/AuthenticatedPacket.h"
#include "packets/UnauthenticatedPacket.h"

void DVR::StaticInit() { sInstance.reset(new DVR()); }

DVR::DVR() : evtIndex(0)
{
    // It has it's own packet and message serializers
    messageSerializer = std::make_shared<MessageSerializer>();

    holistic::SetupMessageSerializer(messageSerializer);

    auto bitReader = std::make_unique<InputMemoryBitStream>();
    auto bitWriter = std::make_unique<OutputMemoryBitStream>();
    auto packetReader = std::make_unique<StructuredDataReader>(std::move(bitReader));
    auto packetWriter = std::make_unique<StructuredDataWriter>(std::move(bitWriter));

    packetSerializer =
        std::make_shared<PacketSerializer>(messageSerializer, std::move(packetReader), std::move(packetWriter));
    holistic::SetupPacketSerializer(packetSerializer);
}

// Parse out any messages that are less that the passed in sequence id and update the
// sequenceId to the last message received
static void parseOutNewMessages(std::shared_ptr<PacketReceivedEvent> pre, std::vector<ReceivedMessage>* messages,
                                uint32_t* lastMessageSequenceId)
{
    std::shared_ptr<ReliableOrderedPacket> rop;

    // packets on packets...Dumb names
    ReceivedPacket* recPacket = &pre->packet;
    auto p = recPacket->packet;
    switch (p->GetClassIdentifier())
    {
    case UnauthenticatedPacket::CLASS_ID:
    case AuthenticatedPacket::CLASS_ID:
        rop = std::static_pointer_cast<ReliableOrderedPacket>(p);
        break;
    default:
        // Only know how to handle messages that are in order for the moment
        // Also don't have any other packets at this time so doesn't matter
        return;
    }

    for (int i = 0; i < rop->numMessages; ++i)
    {
        auto message = (*rop->messages)[i];
        if (message->GetId() < *lastMessageSequenceId)
        {
            continue;
        }

        // First time we've received this message
        ReceivedMessage msg;
        msg.timeRecieved = recPacket->timeRecieved;
        msg.frameRecieved = recPacket->frameRecieved;
        msg.fromAddress = recPacket->fromAddress;
        msg.message = message;

        messages->push_back(msg);
        (*lastMessageSequenceId)++;
    }
}

// Read all the queued packets and do stuff with them
std::vector<ReceivedMessage> DVR::PopMessages(uint32_t currentTime)
{
    std::vector<ReceivedMessage> msgs;
    msgs.reserve(10);

    ReceivedMessage message;
    bool hasMore = this->messageFirstReceivedQueue.peek(message);
    if(message.timeRecieved == 0) {
        INFO("AHAH");
    }

    int i = 0;
    while (hasMore && message.timeRecieved <= currentTime)
    {
        INFO("Poping {} at {}", message.timeRecieved, i);
        msgs.push_back(message);
        this->messageFirstReceivedQueue.pop();
        hasMore = this->messageFirstReceivedQueue.peek(message);
    }

    return msgs;
}

void DVR::PopulateMessageQueue()
{
    auto messages = GetMessages(0, evtIndex);
    for (auto& msg : messages)
    {
        this->messageFirstReceivedQueue.push(msg);
    }
}

void DVR::PacketReceived(std::shared_ptr<Event> packetReceivedEvent)
{
    auto castEvt = std::static_pointer_cast<PacketReceivedEvent>(packetReceivedEvent);
    evts[evtIndex] = castEvt;
    evtIndex++;
}

// Get the first count packets
uint32_t DVR::GetPackets(uint32_t count, ReceivedPacket* outPackets)
{
    int i;
    for (i = 0; i < count; ++i)
    {
        if (i >= evtIndex)
        {
            break;
        }
        std::memcpy(&outPackets[i], &evts[i]->packet, sizeof(outPackets[i]));
    }
    return i;
}

uint32_t DVR::GetPackets(uint32_t from, uint32_t to, uint32_t count, ReceivedPacket* outPackets) { return 0; }

std::vector<ReceivedMessage> DVR::GetMessages(uint32_t from, uint32_t count)
{
    // PIMP: Kind of weird but try to cache the to and from
    uint32_t upTo;
    std::vector<ReceivedMessage> messages;

    if (count >= evtIndex)
    {
        upTo = evtIndex;
        messages.reserve(evtIndex * 3);
    }
    else
    {
        upTo = count;
        messages.reserve(count * 3);
    }

    uint32_t lastMessageSequenceId = 0;
    // Assume 3 messages per packet.
    for (unsigned i = 0; i < upTo; ++i)
    {
        parseOutNewMessages(evts[i], &messages, &lastMessageSequenceId);
    }

    return messages;
}

bool DVR::ReadReceivedPacketsFromFile(std::string fileLoc)
{
    std::ifstream infile;
    infile.open(fileLoc, std::ios::in | std::ios::binary);

    while (!infile.eof())
    {
        ReceivedPacket receivedPacket;

        uint32_t ip4;
        uint16_t port;

        // write the meta data to the file first
        if (!infile.read(reinterpret_cast<char*>(&receivedPacket.timeRecieved), sizeof(receivedPacket.timeRecieved)))
        {
            break;
        }

        if (!infile.read(reinterpret_cast<char*>(&receivedPacket.frameRecieved), sizeof(receivedPacket.frameRecieved)))
        {
            break;
        }

        if (!infile.read(reinterpret_cast<char*>(&ip4), sizeof(ip4)))
        {
            break;
        }

        if (!infile.read(reinterpret_cast<char*>(&port), sizeof(port)))
        {
            break;
        }
        receivedPacket.fromAddress = SocketAddress(ip4, port);

        uint32_t size;

        if (!infile.read(reinterpret_cast<char*>(&size), sizeof(size)))
        {
            break;
        }

        uint8_t buff[9000];

        if (!infile.read(reinterpret_cast<char*>(&buff), size))
        {
            break;
        }

        INFO("Read {} bytes", size);

        std::unique_ptr<std::vector<uint8_t>> vec = std::make_unique<std::vector<uint8_t>>(buff, buff + size);

        auto packets = packetSerializer->ReadPackets(std::move(vec));

        // There should actually only be one
        for (auto& packet : packets)
        {
            receivedPacket.packet = packet;

            auto receivedEvent = std::make_shared<PacketReceivedEvent>();
            receivedEvent->packet = receivedPacket;
            PacketReceived(receivedEvent);
        }
    }
    infile.close();
    PopulateMessageQueue();
    return true;
}

bool DVR::WriteReceivedPacketsToFile(std::string fileLoc)
{
    std::ofstream outfile;
    outfile.open(fileLoc, std::ios::out | std::ios::binary);

    for (uint i = 0; i < evtIndex; ++i)
    {
        auto receivedPacket = &evts[i]->packet;

        uint32_t ip4 = receivedPacket->fromAddress.GetIP4();
        uint16_t port = receivedPacket->fromAddress.GetPort();
        // write the meta data to the file first
        outfile.write(reinterpret_cast<char*>(&receivedPacket->timeRecieved), sizeof(receivedPacket->timeRecieved));
        outfile.write(reinterpret_cast<char*>(&receivedPacket->frameRecieved), sizeof(receivedPacket->frameRecieved));
        outfile.write(reinterpret_cast<char*>(&ip4), sizeof(ip4));
        outfile.write(reinterpret_cast<char*>(&port), sizeof(port));

        const uint8_t* outBuff;
        uint32_t size;
        packetSerializer->WritePacket(receivedPacket->packet, &outBuff, &size);
        outfile.write(reinterpret_cast<char*>(&size), sizeof(size));
        outfile.write(reinterpret_cast<const char*>(outBuff), size);
    }

    outfile.close();

    return true;
}
