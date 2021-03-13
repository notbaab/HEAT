#include "DVRConsoleCommands.h"
#include "DVR.h"

std::string DVRReplayPackets(std::vector<std::string> args) { return "Not implemented yet\n\0"; }

std::string DVRWriteMessages(std::vector<std::string> args)
{
    if (args.size() != 1)
    {
        throw std::runtime_error("Must supply 1 arguments, a file location ");
    }

    std::string fileLoc = args[0];

    DVR::sInstance->WriteReceivedPacketsToFile(fileLoc);
    DVR::sInstance->ReadReceivedPacketsFromFile(fileLoc);
    return "\n\0";
}

std::string DVRGetRecvMessages(std::vector<std::string> args)
{
    auto receivedMessages = DVR::sInstance->GetRecvMessages(0, 4000);

    return "\n\0";
}

std::string DVRGetRecvPackets(std::vector<std::string> args)
{
    const uint32_t max = 2400;
    PacketInfo outPackets[max];
    uint32_t count = DVR::sInstance->GetRecvPackets(max, outPackets);

    for (int i = 0; i < count; ++i)
    {
        outPackets[i].packet->GetClassIdentifier();
    }

    return "\n\0";
}
