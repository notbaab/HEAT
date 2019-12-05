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
    OutputMemoryBitStream() : mBitHead(0)
    {
        mBuffer = std::make_shared<std::vector<uint8_t>>(1500);
        mBitCapacity = 1500 * 8;
    }

    ~OutputMemoryBitStream() {}

    void WriteBits(uint8_t inData, uint32_t inBitCount);
    void WriteBits(const void* inData, uint32_t inBitCount);

    // Get read only pointer into buffer
    const std::shared_ptr<std::vector<uint8_t>> GetBufferPtr() const { return mBuffer; }

    // Get number of bits written to the buffer
    uint32_t GetBitLength() const { return mBitHead; }

    // Get number of bytes written to the buffer
    uint32_t GetByteLength() const { return (mBitHead + 7) >> 3; }

    // Convenient wrapper around write bits for byte aligned data
    void WriteBytes(const void* inData, uint32_t inByteCount)
    {
        WriteBits(inData, inByteCount << 3);
    }

    // TODO: Why is this done differently than the input memory bit stream
    template <typename T>
    void Write(std::vector<T>& inData)
    {
        // // ByteCount is actually wrong, we don't use it at all. Make the
        // // unused variable warning happy but look into removing it.
        // (void)byteCount;
        // The bits of each element.
        uint32_t inBitCount = sizeof(T) * 8;
        for (auto t : inData)
        {
            WriteBits(&t, inBitCount);
        }
    }

    // Generic templatized write for primative values
    template <typename T>
    void Write(T inData, uint32_t inBitCount = sizeof(T) * 8)
    {
        static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value,
                      "Generic Write only supports primitive  data types");
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
    void serialize(std::vector<T>& inData, int byteCount)
    {
        // we don't use byteCount at all. Make the
        // unused variable warning happy but look into removing it more better.
        // Need to do some weird ness to keep the same interface with the
        // input stream
        (void)byteCount;
        Write(inData);
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
    std::shared_ptr<std::vector<uint8_t>> mBuffer;
    uint32_t mBitHead;     // how many bits have data
    uint32_t mBitCapacity; // how many bits the current buffer can hold
};
