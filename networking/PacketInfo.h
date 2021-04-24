#pragma once

// PIMP: Forward declare these?
#include "packets/Packet.h"
#include "sockets/SocketAddress.h"

// struct that holds about when we have received or sent a packet, along with a pointer
// the the packet itself
struct PacketInfo
{
    // Not sure which one we might want
    uint32_t time;
    uint32_t frame;
    SocketAddress address;
    std::shared_ptr<Packet> packet;
};
