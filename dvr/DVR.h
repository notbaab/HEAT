#pragma once

#include "PacketReceivedEvent.h"
#include "PacketSentEvent.h"
#include "datastructures/ThreadSafeQueue.h"
#include "networking/MessageInfo.h"
#include "packets/PacketSerializer.h"

const uint32_t packetBufferSize = 216000;

class DVR
{
  public:
    static inline std::unique_ptr<DVR> sInstance;
    static void StaticInit();

    DVR();

    bool WriteReceivedPacketsToFile(std::string fileLoc);
    bool ReadReceivedPacketsFromFile(std::string fileLoc);

    void PacketReceived(std::shared_ptr<Event> packetReceivedEvent);
    void PacketSent(std::shared_ptr<Event> packetSentEvent);

    // PIMP: These are going to be incredibly slow since it's mem copying all of them
    // individually.
    uint32_t GetSentPackets(uint32_t count, PacketInfo* outPackets);
    uint32_t GetRecvPackets(uint32_t count, PacketInfo* outPackets);

    std::vector<MessageInfo> GetRecvMessages(uint32_t from, uint32_t count);
    std::vector<MessageInfo> GetSentMessages(uint32_t from, uint32_t count);

    std::vector<MessageInfo> PopRecvMessages(uint32_t time);
    std::vector<MessageInfo> PopSentMessages(uint32_t time);

  protected:
    void PopulateMessageQueue();

    ThreadSafeQueue<MessageInfo> messageFirstReceivedQueue;
    ThreadSafeQueue<MessageInfo> messageFirstSentQueue;

    std::shared_ptr<PacketReceivedEvent> recvEvts[packetBufferSize];
    std::shared_ptr<PacketSentEvent> sentEvts[packetBufferSize];
    uint32_t recvEvtsIndex;
    uint32_t sentEvtsIndex;

    std::shared_ptr<PacketSerializer> packetSerializer;
    std::shared_ptr<MessageSerializer> messageSerializer;

    // keep the messages we get in a neat order of when they were first
    // received in a packet. That will correspond to when they are handled
    // ThreadSafeQueue<MessageInfo*> messageFirstReceivedQueue;

};
