#pragma once
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "OutputStream.h"

class OutputMemoryBitStream : public OutputStream
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
    const std::shared_ptr<std::vector<uint8_t>> GetSharedBuffer() const override { return mBuffer; }
    const uint8_t* GetRawBuffer() const override { return mBuffer->data(); }

    // Get number of bits written to the buffer
    uint32_t GetBitLength() const { return mBitHead; }

    // Get number of bytes written to the buffer
    uint32_t GetByteLength() const { return (mBitHead + 7) >> 3; }

    virtual const uint32_t BytesInBuffer() const override { return GetByteLength(); }
    // Convenient wrapper around write bits for byte aligned data
    void WriteBytes(const void* inData, uint32_t inByteCount) { WriteBits(inData, inByteCount << 3); }

    virtual void Write(void* inData, uint32_t byteCount, const char* name) override { WriteBytes(inData, byteCount); }
    virtual void Write(uint64_t inData, const char* name) override { t_Write(inData); }
    virtual void Write(uint32_t inData, const char* name) override { t_Write(inData); }
    virtual void Write(int inData, const char* name) override { t_Write(inData); }
    virtual void Write(float inData, const char* name) override { t_Write(inData); }

    virtual void Write(uint16_t inData, const char* name) override { t_Write(inData); }
    virtual void Write(int16_t inData, const char* name) override { t_Write(inData); }

    virtual void Write(uint8_t inData, const char* name) override { t_Write(inData); }
    virtual void Write(int8_t inData, const char* name) override { t_Write(inData); }
    virtual void Write(bool inData, const char* name) override { t_Write(inData, 1); }

    // Generic templatized write for primitive values
    template <typename T>
    void t_Write(T inData, uint32_t inBitCount = sizeof(T) * 8)
    {
        static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value,
                      "Generic Write only supports primitive  data types");
        WriteBits(&inData, inBitCount);
    }

    // Writer for string data
    virtual void Write(const std::string& inString, const char* name) override
    {
        uint32_t elementCount = static_cast<uint32_t>(inString.size());
        // Need a size for the receiving end
        t_Write(elementCount);
        WriteBytes(inString.data(), elementCount);
    }

    void PrintByteArray();

    void printStream() const;

    virtual void ResetBuffer() override { mBitHead = 0; }

  private:
    std::shared_ptr<std::vector<uint8_t>> mBuffer;
    uint32_t mBitHead;     // how many bits have data
    uint32_t mBitCapacity; // how many bits the current buffer can hold
};
