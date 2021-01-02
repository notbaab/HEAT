#include "PacketManager.h"

PacketManager::PacketManager(std::shared_ptr<PacketSerializer> packetFactory) : m_packetFactory(packetFactory)
{
    assert((65536 % SlidingWindowSize) == 0);
    assert((65536 % MessageSendQueueSize) == 0);
    assert((65536 % MessageReceiveQueueSize) == 0);

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

    m_time = 0.0;

    m_sentPackets->Reset();
    m_receivedPackets->Reset();

    m_sendMessageId = 0;
    m_receiveMessageId = 0;
    m_oldestUnackedMessageId = 0;

    m_messageSendQueue->Reset();
    m_messageSentPackets->Reset();
    m_messageReceiveQueue->Reset();
}

bool PacketManager::CanSendMessage() const { return m_messageSendQueue->IsAvailable(m_sendMessageId); }

// Refactor to QueueMessage since it doesn't send a message, just adds it to the
// queue to be written on the next write packet call
void PacketManager::SendMessage(std::shared_ptr<Message> message)
{
    assert(message);
    assert(CanSendMessage());

    // Insert into queue
    MessageSendQueueEntry* entry = m_messageSendQueue->Insert(m_sendMessageId);

    assert(entry);

    entry->message = message;
    entry->measuredBits = 0;
    entry->timeLastSent = -1.0;

    m_sendMessageId++;
}

// read the message that was serialized into the queue at one point and removes
// it from any references
std::shared_ptr<Message> PacketManager::ReceiveMessage()
{
    MessageReceiveQueueEntry* entry = m_messageReceiveQueue->Find(m_receiveMessageId);
    if (!entry)
    {
        return NULL;
    }

    auto message = entry->message;

    assert(message);
    assert(message->GetId() == m_receiveMessageId);

    m_messageReceiveQueue->Remove(m_receiveMessageId);

    m_receiveMessageId++;

    return message;
}

// Writes up to max messages into the passed in vector.
void PacketManager::ReceiveMessages(std::vector<std::shared_ptr<Message>>& messages, uint16_t max)
{
    auto message = ReceiveMessage();
    uint16_t processed = 0;

    // PIMP Size the vector when we start?
    while (message != NULL && processed <= max)
    {
        messages.emplace_back(message);
        message = ReceiveMessage();
        processed++;
    }
}

// Accepts an id of a packet that HAS to be a subclass of ReliableOrderedPacket
std::shared_ptr<ReliableOrderedPacket> PacketManager::WritePacket(uint32_t packetId)
{
    std::shared_ptr<Packet> reliablePacket = m_packetFactory->CreatePacket(packetId);
    auto packet = std::static_pointer_cast<ReliableOrderedPacket>(reliablePacket);

    // The only thing that ties the sequenceNumber buffer to the packet
    packet->sequenceNumber = m_sentPackets->GetSequence();
    GenerateAckBits(*m_receivedPackets, packet->ack, packet->ackBits);
    InsertAckPacketEntry(packet->sequenceNumber);

    // do this still, but use the ids to point to an array of raw or shared ptrs I think
    int numMessageIds;
    uint16_t messageIds[MaxMessagesPerPacket];
    MessageSendQueueEntry* entries[MaxMessagesPerPacket];

    // This will get the ids of messages that haven't been sent or were sent long
    // ago but we never got an ack for them and put them in the array
    GetMessagesToSend(messageIds, numMessageIds, entries);

    // ties the packet with the sequence number to the ids of those messages so
    // we know if this packet is ack'd the messages were received as well.
    AddMessagePacketEntry(messageIds, numMessageIds, packet->sequenceNumber);

    packet->numMessages = numMessageIds;

    for (int i = 0; i < numMessageIds; ++i)
    {
        MessageSendQueueEntry* entry = entries[i];
        assert(entry && entry->message);

        // write the ids that the serializer will assign to the message on the
        // receiving side.
        packet->messageIds[i] = messageIds[i];
        // push back each message to the packet message vector
        packet->messages->push_back(entry->message);
    }

    return packet;
}

std::vector<std::shared_ptr<Packet>> PacketManager::ConvertBytesToPackets(std::unique_ptr<std::vector<uint8_t>> data)
{
    return m_packetFactory->ReadPackets(std::move(data));
}

// Process the Reliable packet by processing the ack and ackBits of the packet
// and read all the messages
bool PacketManager::ReadPacket(std::shared_ptr<ReliableOrderedPacket> packet)
{
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

// process ack bits of the packet
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

// get any messages that need to be sent by check the last time they were sent
// and comparing it to the resend rate. The last time they were sent is always
// -1 on newly created messages so no special case for messages that we just
// create
void PacketManager::GetMessagesToSend(uint16_t* messageIds, int& numMessageIds, MessageSendQueueEntry** entries)
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

        // TODO: I don't measure the bits so the check there is worthless
        if (entry && (entry->timeLastSent + MessageResendRate <= m_time) && (availableBits - entry->measuredBits >= 0))
        {
            entries[numMessageIds] = entry;
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

void PacketManager::AddMessagePacketEntry(const uint16_t* messageIds, int& numMessageIds, uint16_t sequenceNumber)
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
        {
            sentPacket->messageIds[i] = messageIds[i];
        }
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
        std::shared_ptr<Message> message = (*messageRef)[i];
        assert(message);

        // When reading a message, GetId is valid since it should have been
        // assigned one
        const uint16_t messageId = message->GetId();

        // Already received this message, just waiting for a call to receive messages to
        // dequeue it
        if (m_messageReceiveQueue->Find(messageId))
        {
            continue;
        }

        // The message is too old to be read. This should not happen except for
        // weird cases
        if (sequence_less_than(messageId, minMessageId))
        {
            continue;
        }

        if (sequence_greater_than(messageId, maxMessageId))
        {
            return;
        }

        // Unseen message, add it to the queue
        MessageReceiveQueueEntry* entry = m_messageReceiveQueue->Insert(messageId);

        assert(entry);

        entry->message = message;
    }
}

// Given the ack, look at the sent packet that corresponds to that ack and
// remove any messages from the send queue that were included in that packet
// since they got through
void PacketManager::ProcessMessageAck(uint16_t ack)
{
    MessageSentPacketEntry* sentPacketEntry = m_messageSentPackets->Find(ack);

    if (!sentPacketEntry)
    {
        return;
    }

    assert(!sentPacketEntry->acked);

    for (int i = 0; i < (int)sentPacketEntry->numMessageIds; ++i)
    {
        const uint16_t messageId = sentPacketEntry->messageIds[i];

        MessageSendQueueEntry* sendQueueEntry = m_messageSendQueue->Find(messageId);

        if (sendQueueEntry)
        {
            assert(sendQueueEntry->message);
            TRACE("Got ack for {}, removing", messageId)
            m_messageSendQueue->Remove(messageId);
        }
        else
        {
            TRACE("Already got ack for {}, nothing to do.", messageId)
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
