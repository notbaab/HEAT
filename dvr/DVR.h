#pragma once

#include "PacketReceivedEvent.h"
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
    uint32_t GetPackets(uint32_t count, PacketInfo* outPackets);
    uint32_t GetPackets(uint32_t count, uint32_t start, PacketInfo* outPackets);
    uint32_t GetPackets(uint32_t from, uint32_t to, uint32_t count, PacketInfo* outPackets);

    std::vector<MessageInfo> GetMessages(uint32_t from, uint32_t count);

    std::vector<MessageInfo> PopMessages(uint32_t time);

  protected:
    void PopulateMessageQueue();

    ThreadSafeQueue<MessageInfo> messageFirstReceivedQueue;

    std::shared_ptr<PacketReceivedEvent> evts[packetBufferSize];
    std::shared_ptr<PacketSerializer> packetSerializer;
    std::shared_ptr<MessageSerializer> messageSerializer;

    // keep the messages we get in a neat order of when they were first
    // received in a packet. That will correspond to when they are handled
    // ThreadSafeQueue<MessageInfo*> messageFirstReceivedQueue;

    uint32_t evtIndex;
};
