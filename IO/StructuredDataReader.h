#pragma once

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "InputStream.h"

class StructuredDataReader
{
  public:
    StructuredDataReader(std::unique_ptr<InputStream> underlyingStream) : underlyingStream(std::move(underlyingStream))
    {
    }

    const uint8_t* GetRawBuffer() const { return underlyingStream->GetRawBuffer(); }
    const std::shared_ptr<std::vector<uint8_t>> GetSharedBuffer() const { return underlyingStream->GetSharedBuffer(); }
    uint32_t GetRemainingBitCount() const { return mBitCapacity - mBitHead; }
    uint32_t GetByteCapacity() const { return mBitCapacity >> 3; }

    template <typename T>
    void serialize(T& inData, const char* fieldName)
    {
        underlyingStream->Read(inData, fieldName);
    }

    template <typename T>
    void serialize(T& inData)
    {
        underlyingStream->Read(inData, "TEMP");
    }

    // PIMP: Not byte count but element count and calculate the byte count
    template <typename T>
    void serialize(std::vector<T>& inData, uint32_t byteCount)
    {
        // reserver to the correct amount
        inData.reserve(byteCount);
        underlyingStream->Read(static_cast<void*>(inData.data()), byteCount, "TEMP");
    }

    void SetStreamBuffer(std::shared_ptr<std::vector<uint8_t>> buf)
    {
        underlyingBuffer = buf;
        // PIMP: Both things probably don't need this. Asses later.
        underlyingStream->SetInputBuffer(buf);
    }

    void StartObject() { underlyingStream->StartObject(); }
    void EndObject() { underlyingStream->EndObject(); }
    void StartObject(std::string key) { underlyingStream->StartObject(key); }
    void EndObject(std::string key) { underlyingStream->EndObject(key); }

    void StartArray() { underlyingStream->StartArray(); }
    void EndArray() { underlyingStream->EndArray(); }
    void StartArray(std::string key) { underlyingStream->StartArray(key); }
    void EndArray(std::string key) { underlyingStream->EndArray(key); }

    bool HasMoreData() { return underlyingStream->HasMoreData(); }
    void PrintStream() const;

  private:
    std::unique_ptr<InputStream> underlyingStream;
    std::shared_ptr<std::vector<uint8_t>> underlyingBuffer;
    uint32_t mBitHead;
    uint32_t mBitCapacity;
};
