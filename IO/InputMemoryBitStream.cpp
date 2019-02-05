#include "InputMemoryBitStream.h"
#include "shared.h"
#include <bitset>

// basic bit Read. Reads from mBuffer the number of bits into the single byte
// outData
void InputMemoryBitStream::ReadBits(uint8_t& outData, uint32_t inBitCount)
{
    uint32_t byteOffset = mBitHead >> 3; // how many bytes have we written already
    uint32_t bitOffset = mBitHead & 0x7; // how manay bits have been written

    // Point outData to the correct location in the current buffer by grabing
    // the current byte and shift it by the amount of bits written
    outData = static_cast<uint8_t>(mBuffer->data()[byteOffset]) >> bitOffset;

    // How many bits are actually open to use at this location
    uint32_t bitsFreeThisByte = 8 - bitOffset;

    // If not enough bits free to read,
    if (bitsFreeThisByte < inBitCount)
    {
        // we need another byte, grab the next byte but shift it back the
        // amount of bits that were free.
        outData |= static_cast<uint8_t>(mBuffer->data()[byteOffset + 1]) << bitsFreeThisByte;
    }

    // Mask out the data we don't want, leaving the bits we want in the last
    // LSB
    outData &= (~(0x00ff << inBitCount));

    mBitHead += inBitCount;
}

/**
 * Read any number of bits from the current mBuffer and put them into the
 * void pointer
 */
void InputMemoryBitStream::ReadBits(void* outData, uint32_t inBitCount)
{
    uint8_t* destByte = reinterpret_cast<uint8_t*>(outData);

    // Write all bytes first
    while (inBitCount > 8)
    {
        ReadBits(*destByte, 8);
        ++destByte;
        inBitCount -= 8;
    }

    // If any bits are left, write the rest
    if (inBitCount > 0)
    {
        ReadBits(*destByte, inBitCount);
    }
}

void InputMemoryBitStream::printStream() const
{
    auto streamBuffer = GetBufferPtr();
    uint32_t bufferSize = GetByteCapacity();

    ::printStream(bufferSize, streamBuffer);
}
