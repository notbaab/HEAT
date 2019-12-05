#include "config.h"

#include <epan/dissectors/packet-tcp.h>
#include <epan/packet.h>
#include <epan/prefs.h>
#include <iostream>
#include <string>

#include "packets/MessageSerializer.h"
#include "packets/Message.h"
#include "packets/PacketSerializer.h"
#include "packets/AuthenticatedPacket.h"

#define FAL_PORT 4500

static gint ett_heat = -1;

static int proto_heat = -1;
static int hf_packet_type = -1;
static int hf_packet_sequence = -1;
static int hf_packet_ack = -1;
static int hf_packet_ack_bits = -1;
static int hf_client_salt = -1;
// static int hf_message_type = -1;

// void proto_register_fal(void);
// void proto_reg_handoff_fal(void);

void testC14Comp()
{
    auto identity = [](auto x) { return x; };
    int three = identity(3);           // == 3
    std::string foo = identity("foo"); // == "foo"
    std::cout << three << foo;
}

static int dissect_heat(tvbuff_t* tvb, packet_info* pinfo, proto_tree* tree _U_, void* data _U_)
{
    col_set_str(pinfo->cinfo, COL_PROTOCOL, "HEAT");

    proto_item* ti = proto_tree_add_item(tree, proto_heat, tvb, 0, -1, ENC_NA);
    proto_tree *heat_tree = proto_item_add_subtree(ti, ett_heat);

    int offset = 0;
    int size = 0;

    uint32_t packet_type = tvb_get_letohl(tvb, 0);
    std::string packetStr = Message::StringFromId(packet_type);

    size = 4;
    // proto_tree_add_item(heat_tree, hf_packet_type, tvb, offset, size, ENC_LITTLE_ENDIAN);
    proto_tree_add_string(heat_tree, hf_packet_type, tvb, offset, size, packetStr.c_str());
    offset += size;

    if (packet_type == AuthenticatedPacket::CLASS_ID)
    {
        size = 4;
        proto_tree_add_item(heat_tree, hf_client_salt, tvb, offset, size, ENC_LITTLE_ENDIAN);
        offset += size;
    }

    size = 4;
    proto_tree_add_item(heat_tree, hf_packet_sequence, tvb, offset, size, ENC_LITTLE_ENDIAN);
    offset += size;

    size = 2;
    proto_tree_add_item(heat_tree, hf_packet_ack, tvb, offset, size, ENC_LITTLE_ENDIAN);
    offset += size;

    size = 4;
    proto_tree_add_item(heat_tree, hf_packet_ack_bits, tvb, offset, size, ENC_LITTLE_ENDIAN);
    offset += 4;

    return tvb_captured_length(tvb);
}

// register the plugin so wireshark knows about our protocol
extern "C" void proto_register_fal(void)
{
    proto_heat = proto_register_protocol("Heat Game Protocol", "heat", "ht");

    static hf_register_info hf[] = {
        { &hf_packet_type,
            { "Packet Type", "heat.p_type", FT_STRING, BASE_NONE, NULL, 0x0, NULL, HFILL}
        },
        { &hf_client_salt,
            { "Client Salt", "heat.salt", FT_UINT32, BASE_HEX, NULL, 0x0, NULL, HFILL}
        },
        { &hf_packet_sequence,
            { "Sequence Number", "heat.seq_num", FT_UINT32, BASE_DEC, NULL, 0x0, NULL, HFILL}
        },
        { &hf_packet_ack,
            { "Ack", "heat.ack", FT_UINT16, BASE_DEC, NULL, 0x0, NULL, HFILL}
        },
        { &hf_packet_ack_bits,
            { "Ack Bits", "heat.ack_bits", FT_UINT32, BASE_HEX, NULL, 0x0, NULL, HFILL}
        },
    };

    static gint *ett[] = {
        &ett_heat
    };

    proto_register_field_array(proto_heat, hf, array_length(hf));
    proto_register_subtree_array(ett, array_length(ett));
}

// create the handle that says what function to call when it gets udp packets
// on FAL_PORT
extern "C" void proto_reg_handoff_fal(void)
{
    dissector_handle_t fal_handle;

    auto messageSerializer = std::make_shared<MessageSerializer>();

    fal_handle = create_dissector_handle(dissect_heat, proto_heat);
    dissector_add_uint("udp.port", FAL_PORT, fal_handle);
}
