#pragma once

// PIMP: Forward declare these?
#include "packets/Packet.h"
#include "sockets/SocketAddress.h"

struct ReceivedPacket
{
    // Not sure which one we might want
    uint32_t timeRecieved;
    uint32_t frameRecieved;
    SocketAddress fromAddress;
    std::shared_ptr<Packet> packet;
};
