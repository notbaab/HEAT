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

std::string DVRGetMessages(std::vector<std::string> args)
{
    auto receivedMessages = DVR::sInstance->GetMessages(0, 4000);

    return "\n\0";
}

std::string DVRGetPackets(std::vector<std::string> args)
{
    const uint32_t max = 2400;
    ReceivedPacket outPackets[max];
    uint32_t count = DVR::sInstance->GetPackets(max, outPackets);

    for (int i = 0; i < count; ++i)
    {
        outPackets[i].packet->GetClassIdentifier();
    }

    return "\n\0";
}
