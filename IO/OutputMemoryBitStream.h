#pragma once

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <vector>

/**
 * Class for writing data to a buffer stream
 */
class OutputMemoryBitStream
{
  public:
    OutputMemoryBitStream() : mBuffer(nullptr), mBitHead(0)
    {
        ReallocBuffer(1500 * 8); // init 1500 Byte size buffer
    }

    ~OutputMemoryBitStream() { std::free(mBuffer); }

    void WriteBits(uint8_t inData, uint32_t inBitCount);
    void WriteBits(const void* inData, uint32_t inBitCount);

    // Get read only pointer into buffer
    const char* GetBufferPtr() const { return mBuffer; }

    // Get number of bits written to the buffer
    uint32_t GetBitLength() const { return mBitHead; }

    // Get number of bytes written to the buffer
    uint32_t GetByteLength() const { return (mBitHead + 7) >> 3; }

    // Convient wrapper around write bits for byte aligned data
    void WriteBytes(const void* inData, uint32_t inByteCount)
    {
        WriteBits(inData, inByteCount << 3);
    }

    // Generic templatized write for primative values
    template <typename T>
    void Write(T inData, uint32_t inBitCount = sizeof(T) * 8)
    {
        static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value,
                      "Generic Write only supports primitive data types");
        WriteBits(&inData, inBitCount);
    }

    // Writer for bool data, which should only be 1 bit
    void Write(bool inData) { WriteBits(&inData, 1); }

    // Writer for string data
    void Write(const std::string& inString)
    {
        uint32_t elementCount = static_cast<uint32_t>(inString.size());
        Write(elementCount);
        for (const auto& element : inString)
        {
            Write(element);
        }
    }

    template <typename T>
    void serialize(T inData)
    {
        Write(inData);
    }

    template <typename T>
    void serialize(T inData, int bitCount)
    {
        Write(inData, bitCount);
    }

    void PrintByteArray();

    void printStream() const;

  private:
    void ReallocBuffer(uint32_t inNewBitCapacity);

    char* mBuffer;         // buffer pointer
    uint32_t mBitHead;     // how many bits have data
    uint32_t mBitCapacity; // how many bits the current buffer can hold
};
