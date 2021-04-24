#pragma once

// PIMP: Forward declare these?
#include "packets/Message.h"
#include "sockets/SocketAddress.h"

struct MessageInfo
{
    // Not sure which one we might want
    uint32_t time;
    uint32_t frame;
    SocketAddress address;
    std::shared_ptr<Message> message;
};
