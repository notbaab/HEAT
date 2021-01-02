#pragma once

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "OutputStream.h"

class StructuredDataWriter
{
  public:
    StructuredDataWriter(std::unique_ptr<OutputStream> underlyingStream) : underlyingStream(std::move(underlyingStream))
    {
    }

    const uint8_t* GetRawBuffer() const { return underlyingStream->GetRawBuffer(); }
    const std::shared_ptr<std::vector<uint8_t>> GetSharedBuffer() const { return underlyingStream->GetSharedBuffer(); }
    uint32_t GetByteLength() const { return underlyingStream->BytesInBuffer(); }

    // I don't think this needs to be a reference
    template <typename T>
    void serialize(T& inData, const char* fieldName)
    {
        underlyingStream->Write(inData, fieldName);
    }

    template <typename T>
    void serialize(T inData)
    {
        underlyingStream->Write(inData, "");
    }

    template <typename T>
    void serialize(std::vector<T>& inData, uint32_t byteCount, const char* name)
    {
        underlyingStream->Write(static_cast<void*>(inData.data()), byteCount, name);
    }

    void ResetBuffer()
    {
        // PIMP: Both things probably don't need this. Asses later.
        underlyingStream->ResetBuffer();
    }

    void StartObject() { underlyingStream->StartObject(); }
    void EndObject() { underlyingStream->EndObject(); }
    void StartObject(std::string key) { underlyingStream->StartObject(key); }
    void EndObject(std::string key) { underlyingStream->EndObject(key); }

    void StartArray() { underlyingStream->StartArray(); }
    void EndArray() { underlyingStream->EndArray(); }
    void StartArray(std::string key) { underlyingStream->StartArray(key); }
    void EndArray(std::string key) { underlyingStream->EndArray(key); }

    void PrintStream() const;

  private:
    std::unique_ptr<OutputStream> underlyingStream;
};
