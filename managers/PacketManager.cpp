#include "PacketManager.h"

PacketManager::PacketManager(std::shared_ptr<PacketSerializer> packetFactory)
    : m_packetFactory(packetFactory)
{
    assert((65536 % SlidingWindowSize) == 0);
    assert((65536 % MessageSendQueueSize) == 0);
    assert((65536 % MessageReceiveQueueSize) == 0);

    // m_packetFactory = packetFactory;

    // m_messageFactory = messageFactory;

    // m_error = CONNECTION_ERROR_NONE;

    // m_messageOverheadBits = CalculateMessageOv   erheadBits();
    m_sentPackets = new SequenceBuffer<SentPacketData>(SlidingWindowSize);
    m_receivedPackets = new SequenceBuffer<ReceivedPacketData>(SlidingWindowSize);
    m_messageSendQueue = new SequenceBuffer<MessageSendQueueEntry>(MessageSendQueueSize);
    m_messageSentPackets = new SequenceBuffer<MessageSentPacketEntry>(SlidingWindowSize);
    m_messageReceiveQueue = new SequenceBuffer<MessageReceiveQueueEntry>(MessageReceiveQueueSize);
    m_sentPacketMessageIds = new uint16_t[MaxMessagesPerPacket * MessageSendQueueSize];

    Reset();
}

PacketManager::~PacketManager()
{
    Reset();

    assert(m_sentPackets);
    assert(m_receivedPackets);
    assert(m_messageSendQueue);
    assert(m_messageSentPackets);
    assert(m_messageReceiveQueue);
    assert(m_sentPacketMessageIds);

    delete m_sentPackets;
    delete m_receivedPackets;
    delete m_messageSendQueue;
    delete m_messageSentPackets;
    delete m_messageReceiveQueue;
    delete[] m_sentPacketMessageIds;

    m_sentPackets = NULL;
    m_receivedPackets = NULL;
    m_messageSendQueue = NULL;
    m_messageSentPackets = NULL;
    m_messageReceiveQueue = NULL;
    m_sentPacketMessageIds = NULL;
}

void PacketManager::Reset()
{
    // m_error = CONNECTION_ERROR_NONE;

    m_time = 0.0;

    m_sentPackets->Reset();
    m_receivedPackets->Reset();

    m_sendMessageId = 0;
    m_receiveMessageId = 0;
    m_oldestUnackedMessageId = 0;

    // for (int i = 0; i < m_messageSendQueue->GetSize(); ++i)
    // {
    //     MessageSendQueueEntry* entry = m_messageSendQueue->GetAtIndex(i);
    //     if (entry && entry->message)
    //         entry->message->Release();
    // }

    // for (int i = 0; i < m_messageReceiveQueue->GetSize(); ++i)
    // {
    //     MessageReceiveQueueEntry* entry = m_messageReceiveQueue->GetAtIndex(i);
    //     if (entry && entry->message)
    //         entry->message->Release();
    // }

    m_messageSendQueue->Reset();
    m_messageSentPackets->Reset();
    m_messageReceiveQueue->Reset();
}

bool PacketManager::CanSendMessage() const
{
    return m_messageSendQueue->IsAvailable(m_sendMessageId);
}

// Make this queue message
void PacketManager::SendMessage(std::shared_ptr<Message> message)
{
    assert(message);
    assert(CanSendMessage());


    message->AssignId(m_sendMessageId);

    // Insert into queue
    MessageSendQueueEntry* entry = m_messageSendQueue->Insert(m_sendMessageId);

    assert(entry);

    entry->message = message;
    entry->measuredBits = 0;
    entry->timeLastSent = -1.0;

    // MeasureStream measureStream(MessagePacketBudget / 2);

    // message->SerializeInternal(measureStream);

    // if (measureStream.GetError())
    // {
    //     m_error = CONNECTION_ERROR_MESSAGE_SERIALIZE_MEASURE_FAILED;
    //     message->Release();
    //     return;
    // }

    // entry->measuredBits = measureStream.GetBitsProcessed() + m_messageOverheadBits;

    m_sendMessageId++;
}

// read the message that was serialized into the queue at one point
// This is where we will start to do something else
Message* PacketManager::ReceiveMessage()
{
    // if (GetError() != CONNECTION_ERROR_NONE)
    //     return NULL;

    MessageReceiveQueueEntry* entry = m_messageReceiveQueue->Find(m_receiveMessageId);
    if (!entry)
        return NULL;

    Message* message = entry->message;

    assert(message);
    assert(message->GetId() == m_receiveMessageId);

    m_messageReceiveQueue->Remove(m_receiveMessageId);

    m_receiveMessageId++;

    return message;
}

std::shared_ptr<ReliableOrderedPacket> PacketManager::WritePacket()
{
    // if (m_error != CONNECTION_ERROR_NONE)
    //     return NULL;

    // ConnectionPacket* packet =
    // (ConnectionPacket*)m_packetFactory->CreatePacket(CONNECTION_PACKET);

    std::shared_ptr<Packet> reliablePacket =
        m_packetFactory->CreatePacket(ReliableOrderedPacket::ID);
    auto packet = std::static_pointer_cast<ReliableOrderedPacket>(reliablePacket);

    // if (!packet)
    //     return NULL;

    // The only thing that ties the sequenceNumber buffer to the packet
    packet->sequenceNumber = m_sentPackets->GetSequence();
    GenerateAckBits(*m_receivedPackets, packet->ack, packet->ackBits);
    InsertAckPacketEntry(packet->sequenceNumber);

    // do this still, but use the ids to point to an array of raw or shared ptrs I think
    int numMessageIds;
    uint16_t messageIds[MaxMessagesPerPacket];

    // This will get the ids of messages that aren't un acked and put them
    // in the array
    GetMessagesToSend(messageIds, numMessageIds);

    // Might be still needed,
    AddMessagePacketEntry(messageIds, numMessageIds, packet->sequenceNumber);

    packet->numMessages = numMessageIds;

    for (int i = 0; i < numMessageIds; ++i)
    {
        MessageSendQueueEntry* entry = m_messageSendQueue->Find(messageIds[i]);
        assert(entry && entry->message);
        // pushe back each message to the packet message vector
        packet->messages->push_back(entry->message);
    }

    return packet;
}

bool PacketManager::ReadPacket(std::shared_ptr<ReliableOrderedPacket> packet)
{
    // if (m_error != CONNECTION_ERROR_NONE)
    //     return false;

    // assert(packet);
    // assert(packet->GetType() == CONNECTION_PACKET);

    ProcessAcks(packet->ack, packet->ackBits);
    ProcessPacketMessages(packet.get());
    m_receivedPackets->Insert(packet->sequenceNumber);
    return true;
}

// Keep track of in flight packets
void PacketManager::InsertAckPacketEntry(uint16_t sequenceNumber)
{
    SentPacketData* entry = m_sentPackets->Insert(sequenceNumber);

    assert(entry);

    if (entry)
    {
        entry->acked = 0;
    }
}

void PacketManager::ProcessAcks(uint16_t ack, uint32_t ack_bits)
{
    for (int i = 0; i < 32; ++i)
    {
        if (ack_bits & 1)
        {
            const uint16_t sequenceNumber = ack - i;
            SentPacketData* packetData = m_sentPackets->Find(sequenceNumber);
            if (packetData && !packetData->acked)
            {
                ProcessMessageAck(sequenceNumber);
                packetData->acked = 1;
            }
        }

        ack_bits >>= 1;
    }
}

void PacketManager::GetMessagesToSend(uint16_t* messageIds, int& numMessageIds)
{
    numMessageIds = 0;

    if (m_oldestUnackedMessageId == m_sendMessageId)
        return;

#if _DEBUG
    MessageSendQueueEntry* firstEntry = m_messageSendQueue->Find(m_oldestUnackedMessageId);
    assert(firstEntry);
#endif // #if _DEBUG

    const int GiveUpBits = 8 * 8;
    int availableBits = MessagePacketBudget * 8;
    const int messageLimit = std::min(MessageSendQueueSize, MessageReceiveQueueSize) / 2;

    for (int i = 0; i < messageLimit; ++i)
    {
        const uint16_t messageId = m_oldestUnackedMessageId + i;
        MessageSendQueueEntry* entry = m_messageSendQueue->Find(messageId);

        if (entry && (entry->timeLastSent + MessageResendRate <= m_time) &&
            (availableBits - entry->measuredBits >= 0))
        {
            messageIds[numMessageIds++] = messageId;
            entry->timeLastSent = m_time;
            availableBits -= entry->measuredBits;
        }

        if (availableBits <= GiveUpBits)
            break;

        if (numMessageIds == MaxMessagesPerPacket)
            break;
    }
}

void PacketManager::AddMessagePacketEntry(const uint16_t* messageIds, int& numMessageIds,
                                          uint16_t sequenceNumber)
{
    MessageSentPacketEntry* sentPacket = m_messageSentPackets->Insert(sequenceNumber);

    assert(sentPacket);

    if (sentPacket)
    {
        sentPacket->acked = 0;
        sentPacket->timeSent = m_time;

        const int sentPacketIndex = m_sentPackets->GetIndex(sequenceNumber);

        sentPacket->messageIds = &m_sentPacketMessageIds[sentPacketIndex * MaxMessagesPerPacket];
        sentPacket->numMessageIds = numMessageIds;
        for (int i = 0; i < numMessageIds; ++i)
            sentPacket->messageIds[i] = messageIds[i];
    }
}

void PacketManager::ProcessPacketMessages(const ReliableOrderedPacket* packet)
{
    const uint16_t minMessageId = m_receiveMessageId;
    const uint16_t maxMessageId = m_receiveMessageId + MessageReceiveQueueSize - 1;

    // TODO: Get better data types to avoid this shananegains
    auto messageRef = packet->messages;

    for (int i = 0; i < packet->numMessages; ++i)
    {
        auto message = (*messageRef)[i].get();
        assert(message);
        const uint16_t messageId = message->GetId();

        if (m_messageReceiveQueue->Find(messageId))
            continue;

        if (sequence_less_than(messageId, minMessageId))
            continue;

        if (sequence_greater_than(messageId, maxMessageId))
        {
            // m_error = CONNECTION_ERROR_MESSAGE_DESYNC;
            return;
        }

        MessageReceiveQueueEntry* entry = m_messageReceiveQueue->Insert(messageId);

        assert(entry);

        if (entry)
        {
            entry->message = message;
        }
    }
}

void PacketManager::ProcessMessageAck(uint16_t ack)
{
    MessageSentPacketEntry* sentPacketEntry = m_messageSentPackets->Find(ack);

    if (!sentPacketEntry)
        return;

    assert(!sentPacketEntry->acked);

    for (int i = 0; i < (int)sentPacketEntry->numMessageIds; ++i)
    {
        const uint16_t messageId = sentPacketEntry->messageIds[i];

        MessageSendQueueEntry* sendQueueEntry = m_messageSendQueue->Find(messageId);

        if (sendQueueEntry)
        {
            assert(sendQueueEntry->message);
            assert(sendQueueEntry->message->GetId() == messageId);

            m_messageSendQueue->Remove(messageId);
        }
    }

    UpdateOldestUnackedMessageId();
}

void PacketManager::UpdateOldestUnackedMessageId()
{
    const uint16_t stopMessageId = m_messageSendQueue->GetSequence();

    while (true)
    {
        if (m_oldestUnackedMessageId == stopMessageId)
            break;

        MessageSendQueueEntry* entry = m_messageSendQueue->Find(m_oldestUnackedMessageId);
        if (entry)
            break;

        ++m_oldestUnackedMessageId;
    }

    assert(!sequence_greater_than(m_oldestUnackedMessageId, stopMessageId));
}

void PacketManager::SetTime(double time)
{
    m_time = time;

    // clean up
    m_sentPackets->RemoveOldEntries();
    m_receivedPackets->RemoveOldEntries();
    m_messageSentPackets->RemoveOldEntries();
}
