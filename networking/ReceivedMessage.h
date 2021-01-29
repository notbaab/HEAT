#pragma once

// PIMP: Forward declare these?
#include "packets/Message.h"
#include "sockets/SocketAddress.h"

struct ReceivedMessage
{
    // Not sure which one we might want
    uint32_t timeRecieved;
    uint32_t frameRecieved;
    SocketAddress fromAddress;
    std::shared_ptr<Message> message;
};
