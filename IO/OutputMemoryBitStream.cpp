#include "OutputMemoryBitStream.h"
#include "shared.h"

void OutputMemoryBitStream::WriteBits(uint8_t inData, uint32_t inBitCount)
{
    uint32_t nextBitHead = mBitHead + static_cast<uint32_t>(inBitCount);

    if (nextBitHead > mBitCapacity)
    {
        // ReallocBuffer(std::max(mBitCapacity * 2, nextBitHead));
        mBitCapacity = std::max(mBitCapacity * 2, nextBitHead);
        mBuffer->resize(mBitCapacity << 3);
    }

    // calculate the byteOffset into our buffer
    // by dividing the head by 8
    // and the bitOffset by taking the last 3 bits
    uint32_t byteOffset = mBitHead >> 3;
    uint32_t bitOffset = mBitHead & 0x7;

    uint8_t currentMask = ~(0xff << bitOffset);
    mBuffer->data()[byteOffset] = (mBuffer->data()[byteOffset] & currentMask) | (inData << bitOffset);

    // calculate how many bits were not yet used in
    // our target byte in the buffer
    uint32_t bitsFreeThisByte = 8 - bitOffset;

    // if we needed more than that, carry to the next byte
    if (bitsFreeThisByte < inBitCount)
    {
        // we need another byte
        mBuffer->data()[byteOffset + 1] = inData >> bitsFreeThisByte;
    }

    mBitHead = nextBitHead;
}

void OutputMemoryBitStream::WriteBits(const void* inData, uint32_t inBitCount)
{
    const auto* srcByte = static_cast<const uint8_t*>(inData);
    // write all the bytes
    while (inBitCount > 8)
    {
        WriteBits(*srcByte, 8);
        ++srcByte;
        inBitCount -= 8;
    }
    // write anything left
    if (inBitCount > 0)
    {
        WriteBits(*srcByte, inBitCount);
    }
}
