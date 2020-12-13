#include <bitset>

#include "shared.h"

void printStream(uint32_t bufferSize, const std::shared_ptr<std::vector<uint8_t>> streamBuffer)
{
    printf("Hex: ");
    for (int charIdx = 0; charIdx < bufferSize; charIdx++)
    {
        printf("%x", (*streamBuffer)[charIdx]);
    }

    for (int charIdx = 0; charIdx < bufferSize; charIdx++)
    {
        std::bitset<8> asBits((*streamBuffer)[charIdx]);
        std::string asString = asBits.to_string<char, std::string::traits_type, std::string::allocator_type>();

        printf("%s", asString.c_str());
    }
}

void printStream(uint32_t bufferSize, const char* streamBuffer)
{
    printf("Hex: ");
    for (int charIdx = 0; charIdx < bufferSize; charIdx++)
    {
        printf("%x", streamBuffer[charIdx]);
    }

    for (int charIdx = 0; charIdx < bufferSize; charIdx++)
    {
        std::bitset<8> asBits(streamBuffer[charIdx]);
        std::string asString = asBits.to_string<char, std::string::traits_type, std::string::allocator_type>();

        printf("%s", asString.c_str());
    }
}