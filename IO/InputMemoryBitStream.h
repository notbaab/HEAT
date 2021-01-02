#pragma once

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "InputStream.h"

class InputMemoryBitStream : public InputStream
{
  public:
    InputMemoryBitStream() : InputMemoryBitStream(std::make_shared<std::vector<uint8_t>>()) {}

    InputMemoryBitStream(std::shared_ptr<std::vector<uint8_t>> inBuffer)
        : InputMemoryBitStream(inBuffer, inBuffer->size() * 8.0)
    {
    }

    InputMemoryBitStream(std::shared_ptr<std::vector<uint8_t>> inBuffer, uint32_t inBitCount)
        : mBuffer(inBuffer), mBitHead(0), mBitCapacity(inBitCount)
    {
    }

    // Kinda a crappy constructor that results in copying the entire buffer but it's a sacrifice for
    // safety
    InputMemoryBitStream(uint8_t* inBuffPtr, uint32_t inByteCount)
        : mBuffer(std::make_shared<std::vector<uint8_t>>(inBuffPtr, inBuffPtr + inByteCount)), mBitHead(0),
          mBitCapacity(inByteCount * 8)
    {
    }

    InputMemoryBitStream(const InputMemoryBitStream& inOther) { mBuffer = inOther.mBuffer; }

    virtual bool HasMoreData() override { return GetRemainingBitCount() > 0; }

    uint32_t GetRemainingBitCount() const { return mBitCapacity - mBitHead; }
    uint32_t GetByteCapacity() const { return mBitCapacity >> 3; }

    void ReadBits(uint8_t& outData, uint32_t inBitCount);
    void ReadBits(void* outData, uint32_t inBitCount);

    void ReadBytes(void* outData, uint32_t inByteCount) { ReadBits(outData, inByteCount << 3); }

    template <typename T>
    void t_Read(T& inData, uint32_t inBitCount = sizeof(T) * 8)
    {
        static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value,
                      "Generic Read only supports primitive data types");
        ReadBits(&inData, inBitCount);
    }

    virtual void Read(void* outData, uint32_t byteCount, const char* name) override { ReadBytes(outData, byteCount); }
    virtual void Read(uint64_t& outData, const char* name) override { t_Read(outData); }
    virtual void Read(uint32_t& outData, const char* name) override { t_Read(outData); }
    virtual void Read(int& outData, const char* name) override { t_Read(outData); }
    virtual void Read(float& outData, const char* name) override { t_Read(outData); }

    virtual void Read(uint16_t& outData, const char* name) override { t_Read(outData); }
    virtual void Read(int16_t& outData, const char* name) override { t_Read(outData); }

    virtual void Read(uint8_t& outData, const char* name) override { t_Read(outData); }
    virtual void Read(int8_t& outData, const char* name) override { t_Read(outData); }
    virtual void Read(bool& outData, const char* name) override { t_Read(outData, 1); }

    // This seems...dumb
    virtual void Read(std::string& inString, const char* name) override
    {
        uint32_t elementCount;
        t_Read(elementCount);
        inString.resize(elementCount);
        ReadBytes(inString.data(), elementCount);
    }

    void printStream() const;

    virtual void SetInputBuffer(std::shared_ptr<std::vector<uint8_t>> buf) override
    {
        mBuffer = buf;
        mBitCapacity = mBuffer->size() * 8.0;
        mBitHead = 0;
    }

  private:
    std::shared_ptr<std::vector<uint8_t>> mBuffer;
    uint32_t mBitHead;
    uint32_t mBitCapacity;
};
