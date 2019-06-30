#pragma once

#include <unordered_map>

#include "SequenceBuffer.h"
#include "packets/Message.h"
#include "packets/MessageSerializer.h"
#include "packets/PacketSerializer.h"
#include "packets/ReliableOrderedPacket.h"

const uint32_t ProtocolId = 0x12311616;
const int MaxPacketSize = 4096;
const int MaxMessagesPerPacket = 64;
const int SlidingWindowSize = 1024;
const int MessageSendQueueSize = 1024;
const int MessageReceiveQueueSize = 256;
const int MessagePacketBudget = 1024;
const float MessageResendRate = 0.1f;

class PacketManager
{
  public:
    PacketManager(std::shared_ptr<PacketSerializer> packetFactory);
    ~PacketManager();
    void Reset();
    bool CanSendMessage() const;
    void SendMessage(std::shared_ptr<Message> message);
    Message* ReceiveMessage();
    std::shared_ptr<ReliableOrderedPacket> WritePacket();
    bool ReadPacket(std::shared_ptr<ReliableOrderedPacket> packet);

    std::vector<std::shared_ptr<Packet>>
    ConvertBytesToPackets(std::unique_ptr<std::vector<uint8_t>> data);

    void SetTime(double time);
    // ConnectionError GetError() const;

    // packet factory for creating and destroying connection packets
    std::shared_ptr<PacketSerializer> m_packetFactory;

  protected:
    struct SentPacketData
    {
        uint8_t acked;
    };

    struct ReceivedPacketData
    {
    };

    struct MessageSendQueueEntry
    {
        std::shared_ptr<Message> message;
        double timeLastSent;
        int measuredBits;
    };

    struct MessageSentPacketEntry
    {
        double timeSent;
        uint16_t* messageIds;
        uint32_t numMessageIds : 16; // number of messages in this packet
        uint32_t acked : 1;          // 1 if this sent packet has been acked
    };

    struct MessageReceiveQueueEntry
    {
        Message* message;
    };

    void InsertAckPacketEntry(uint16_t sequence);
    void ProcessAcks(uint16_t ack, uint32_t ack_bits);
    void GetMessagesToSend(uint16_t* messageIds, int& numMessageIds);
    void AddMessagePacketEntry(const uint16_t* messageIds, int& numMessageIds, uint16_t sequence);
    void ProcessPacketMessages(const ReliableOrderedPacket* packet);
    void ProcessMessageAck(uint16_t ack);
    void UpdateOldestUnackedMessageId();

    // int CalculateMessageOverheadBits();

  private:
    MessageSerializer m_messageFactory; // message factory creates and destroys messages
    double m_time;                      // current connection time

    // ConnectionError m_error; // connection error level
    SequenceBuffer<SentPacketData>* m_sentPackets; // sequence buffer of recently sent packets
    // sequence buffer of recently received packets
    SequenceBuffer<ReceivedPacketData>* m_receivedPackets;
    SequenceBuffer<MessageSendQueueEntry>* m_messageSendQueue; // message send queue
    SequenceBuffer<MessageSentPacketEntry>*
        m_messageSentPackets; // messages in sent packets (for acks)
    SequenceBuffer<MessageReceiveQueueEntry>* m_messageReceiveQueue; // message receive queue

    int m_messageOverheadBits;         // number of bits overhead per-serialized message
    uint16_t m_sendMessageId;          // id for next message added to send queue
    uint16_t m_receiveMessageId;       // id for next message to be received
    uint16_t m_oldestUnackedMessageId; // id for oldest unacked message in send queue
    uint16_t* m_sentPacketMessageIds;  // array of message ids, n ids per-sent packet
};
