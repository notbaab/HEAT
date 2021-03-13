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

DVR::DVR() : recvEvtsIndex(0), sentEvtsIndex(0)
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

static void parseOutNewMessages(PacketInfo* packetInfo, std::vector<MessageInfo>* messages,
                                uint32_t* lastMessageSequenceId)
{
    std::shared_ptr<ReliableOrderedPacket> rop;

    // packets on packets...Dumb names
    auto p = packetInfo->packet;
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
        MessageInfo msg;
        msg.time = packetInfo->time;
        msg.frame = packetInfo->frame;
        msg.address = packetInfo->address;
        msg.message = message;

        messages->push_back(msg);
        (*lastMessageSequenceId)++;
    }
}

// Parse out any messages that are less that the passed in sequence id and update the
// sequenceId to the last message received
static void parseOutNewMessages(std::shared_ptr<PacketReceivedEvent> pre, std::vector<MessageInfo>* messages,
                                uint32_t* lastMessageSequenceId)
{
    std::shared_ptr<ReliableOrderedPacket> rop;

    // packets on packets...Dumb names
    PacketInfo* recPacket = &pre->packet;
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
        MessageInfo msg;
        msg.time = recPacket->time;
        msg.frame = recPacket->frame;
        msg.address = recPacket->address;
        msg.message = message;

        messages->push_back(msg);
        (*lastMessageSequenceId)++;
    }
}

static std::vector<MessageInfo> popMessages(uint32_t currentTime, ThreadSafeQueue<MessageInfo>* queue)
{
    std::vector<MessageInfo> msgs;
    msgs.reserve(10);

    MessageInfo message;
    bool hasMore = queue->peek(message);
    if(message.time == 0) {
        INFO("AHAH");
    }

    int i = 0;
    while (hasMore && message.time <= currentTime)
    {
        INFO("Poping {} at {}", message.time, i);
        msgs.push_back(message);
        queue->pop();
        hasMore = queue->peek(message);
    }

    return msgs;
}

// Read all the queued packets and do stuff with them
std::vector<MessageInfo> DVR::PopRecvMessages(uint32_t currentTime)
{
    return popMessages(currentTime, &this->messageFirstReceivedQueue);
}

void DVR::PopulateMessageQueue()
{
    auto messages = GetRecvMessages(0, recvEvtsIndex);
    for (auto& msg : messages)
    {
        this->messageFirstReceivedQueue.push(msg);
    }
}

void DVR::PacketReceived(std::shared_ptr<Event> packetReceivedEvent)
{
    auto castEvt = std::static_pointer_cast<PacketReceivedEvent>(packetReceivedEvent);
    recvEvts[recvEvtsIndex] = castEvt;
    recvEvtsIndex++;
}

void DVR::PacketSent(std::shared_ptr<Event> packetSentEvent)
{
    auto castEvt = std::static_pointer_cast<PacketSentEvent>(packetSentEvent);
    sentEvts[sentEvtsIndex] = castEvt;
    sentEvtsIndex++;
}

template <typename T>
static uint32_t getPackets(uint32_t from, uint32_t count, uint32_t lastIndex, T packetEvt, PacketInfo* outPackets) 
{
    int i;
    for (i = 0; i < count; ++i)
    {
        if (i >= lastIndex)
        {
            break;
        }
        std::memcpy(&outPackets[i], &packetEvt[i]->packet, sizeof(outPackets[i]));
    }
    return i;
}


uint32_t DVR::GetRecvPackets(uint32_t count, PacketInfo* outPackets)
{
    return getPackets<std::shared_ptr<PacketReceivedEvent>[]>(0, count, recvEvtsIndex, recvEvts, outPackets);
}


uint32_t DVR::GetSentPackets(uint32_t count, PacketInfo* outPackets)
{
    return getPackets<std::shared_ptr<PacketSentEvent>[]>(0, count, sentEvtsIndex, sentEvts, outPackets);
}


template <typename T>
static std::vector<MessageInfo> getMessages(uint32_t from, uint32_t count, uint32_t lastIndex, T packetEvt) 
{
    uint32_t upTo;
    std::vector<MessageInfo> messages;

    if (count >= lastIndex)
    {
        upTo = lastIndex;
        messages.reserve(lastIndex * 3);
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
        parseOutNewMessages(&packetEvt[i]->packet, &messages, &lastMessageSequenceId);
    }

    return messages;
}


std::vector<MessageInfo> DVR::GetSentMessages(uint32_t from, uint32_t count)
{
    return getMessages<std::shared_ptr<PacketSentEvent>[]>(from, count, sentEvtsIndex, sentEvts);
}


std::vector<MessageInfo> DVR::GetRecvMessages(uint32_t from, uint32_t count)
{
    return getMessages<std::shared_ptr<PacketReceivedEvent>[]>(from, count, recvEvtsIndex, recvEvts);
}


static std::vector<PacketInfo> readPacketFromFile(std::string fileLoc, PacketSerializer* packetSerializer) 
{
    std::vector<PacketInfo> packetInfos;

    std::ifstream infile;
    infile.open(fileLoc, std::ios::in | std::ios::binary);
    while (!infile.eof())
    {
        PacketInfo packetInfo;

        uint32_t ip4;
        uint16_t port;

        // write the meta data to the file first
        if (!infile.read(reinterpret_cast<char*>(&packetInfo.time), sizeof(packetInfo.time)))
        {
            break;
        }

        if (!infile.read(reinterpret_cast<char*>(&packetInfo.frame), sizeof(packetInfo.frame)))
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
        packetInfo.address = SocketAddress(ip4, port);

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
            packetInfo.packet = packet;
            packetInfos.push_back(packetInfo);
        }
    }

    infile.close();
    return packetInfos;
}

bool DVR::ReadReceivedPacketsFromFile(std::string fileLoc)
{
    auto receivedPackets = readPacketFromFile(fileLoc, packetSerializer.get());
    for (auto& packet : receivedPackets)
    {
        auto receivedEvent = std::make_shared<PacketReceivedEvent>();
        receivedEvent->packet = packet;
        PacketReceived(receivedEvent);
    }

    PopulateMessageQueue();
    return true;
}

bool DVR::WriteReceivedPacketsToFile(std::string fileLoc)
{
    std::ofstream outfile;
    outfile.open(fileLoc, std::ios::out | std::ios::binary);

    for (uint i = 0; i < recvEvtsIndex; ++i)
    {
        auto receivedPacket = &recvEvts[i]->packet;

        uint32_t ip4 = receivedPacket->address.GetIP4();
        uint16_t port = receivedPacket->address.GetPort();
        // write the meta data to the file first
        outfile.write(reinterpret_cast<char*>(&receivedPacket->time), sizeof(receivedPacket->time));
        outfile.write(reinterpret_cast<char*>(&receivedPacket->frame), sizeof(receivedPacket->frame));
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
