#pragma once

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <vector>

class InputMemoryBitStream
{
  public:
    InputMemoryBitStream(std::shared_ptr<std::vector<uint8_t>> inBuffer)
        : InputMemoryBitStream(inBuffer, inBuffer->size() * 8)
    {
    }

    InputMemoryBitStream(std::shared_ptr<std::vector<uint8_t>> inBuffer, uint32_t inBitCount)
        : mBuffer(inBuffer), mBitHead(0), mBitCapacity(inBitCount)
    {
    }

    InputMemoryBitStream(const InputMemoryBitStream& inOther) { mBuffer = inOther.mBuffer; }

    const std::shared_ptr<std::vector<uint8_t>> GetBufferPtr() const { return mBuffer; }
    uint32_t GetRemainingBitCount() const { return mBitCapacity - mBitHead; }
    uint32_t GetByteCapacity() const { return mBitCapacity >> 3; }

    void ReadBits(uint8_t& outData, uint32_t inBitCount);
    void ReadBits(void* outData, uint32_t inBitCount);

    void ReadBytes(void* outData, uint32_t inByteCount) { ReadBits(outData, inByteCount << 3); }

    bool serialize_bits(uint32_t& outData, uint32_t inBitCount = 32);

    template <typename T>
    void Read(T& inData, uint32_t inBitCount = sizeof(T) * 8)
    {
        static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value,
                      "Generic Read only supports primitive data types");
        ReadBits(&inData, inBitCount);
    }

    template <typename T>
    void serialize(T& inData)
    {
        Read(inData);
    }

    template <typename T>
    void serialize(std::vector<T>& inData, uint32_t byteCount)
    {
        Read(inData, byteCount);
    }

    template <typename T>
    void serialize(T& inData, int bitCount)
    {
        Read(inData, bitCount);
    }

    void Read(uint32_t& outData, uint32_t inBitCount = 32) { ReadBits(&outData, inBitCount); }
    void Read(int& outData, uint32_t inBitCount = 32) { ReadBits(&outData, inBitCount); }
    void Read(float& outData) { ReadBits(&outData, 32); }

    void Read(uint16_t& outData, uint32_t inBitCount = 16) { ReadBits(&outData, inBitCount); }
    void Read(int16_t& outData, uint32_t inBitCount = 16) { ReadBits(&outData, inBitCount); }

    void Read(uint8_t& outData, uint32_t inBitCount = 8) { ReadBits(&outData, inBitCount); }
    void Read(bool& outData) { ReadBits(&outData, 1); }

    template <typename T>
    void Read(std::vector<T>& inData, uint32_t byteCount)
    {
        // reserver to the correct amount
        inData.reserve(byteCount);
        ReadBytes(static_cast<void*>(inData.data()), byteCount);
    }

    void ResetToCapacity(uint32_t inByteCapacity)
    {
        mBitCapacity = inByteCapacity << 3;
        mBitHead = 0;
    }

    // This seems...dumb
    void Read(std::string& inString)
    {
        uint32_t elementCount;
        Read(elementCount);
        inString.resize(elementCount);
        for (auto& element : inString)
        {
            Read(element);
        }
    }

    void printStream() const;

  private:
    std::shared_ptr<std::vector<uint8_t>> mBuffer;
    uint32_t mBitHead;
    uint32_t mBitCapacity;
};
