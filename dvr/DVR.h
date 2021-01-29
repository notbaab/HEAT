#pragma once

#include "PacketReceivedEvent.h"
#include "datastructures/ThreadSafeQueue.h"
#include "networking/ReceivedMessage.h"
#include "packets/PacketSerializer.h"

const uint32_t packetBufferSize = 216000;

class DVR
{
  public:
    static inline std::unique_ptr<DVR> sInstance;
    static void StaticInit();

    DVR();

    bool WriteReceivedPacketsToFile(std::string fileLoc);
    bool ReadRecording(std::string fileLoc);
    bool ReadReceivedPacketsFromFile(std::string fileLoc);

    void PacketRecieved(std::shared_ptr<Event> packetReceivedEvent);

    // PIMP: These are going to be incredibly slow since it's mem copying all of them
    // individually.
    uint32_t GetPackets(uint32_t count, ReceivedPacket* outPackets);
    uint32_t GetPackets(uint32_t count, uint32_t start, ReceivedPacket* outPackets);
    uint32_t GetPackets(uint32_t from, uint32_t to, uint32_t count, ReceivedPacket* outPackets);

    std::vector<ReceivedMessage> GetMessages(uint32_t from, uint32_t count);

    std::vector<ReceivedMessage> PopMessages(uint32_t time);

  protected:
    std::shared_ptr<PacketReceivedEvent> evts[packetBufferSize];
    std::shared_ptr<PacketSerializer> packetSerializer;
    std::shared_ptr<MessageSerializer> messageSerializer;

    // keep the messages we get in a neat order of when they were first
    // received in a packet. That will correspond to when they are handled
    ThreadSafeQueue<ReceivedMessage*> messageFirstReceivedQueue;

    uint32_t evtIndex;
};