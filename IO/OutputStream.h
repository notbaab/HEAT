#pragma once

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <string>
#include <vector>

class OutputStream
{
  public:
    OutputStream(){};
    virtual ~OutputStream() = default;

    // Write some raw bytes out of the underlying buffer. I don't think I actually want this

    virtual void Write(uint64_t inData, const char* name) = 0;
    virtual void Write(uint32_t inData, const char* name) = 0;
    virtual void Write(int inData, const char* name) = 0;
    virtual void Write(float inData, const char* name) = 0;

    virtual void Write(void* inData, uint32_t byteCount, const char* name) = 0;

    virtual void Write(uint16_t inData, const char* name) = 0;
    virtual void Write(int16_t inData, const char* name) = 0;

    virtual void Write(uint8_t inData, const char* name) = 0;
    virtual void Write(int8_t inData, const char* name) = 0;
    virtual void Write(bool inData, const char* name) = 0;

    virtual void Write(const std::string& inString, const char* name) = 0;

    virtual void ResetBuffer() = 0;
    //
    virtual const std::shared_ptr<std::vector<uint8_t>> GetSharedBuffer() const = 0;
    virtual const uint8_t* GetRawBuffer() const = 0;
    virtual const uint32_t BytesInBuffer() const = 0;

    // Don't need to support object reads and array reads
    virtual void StartObject(){};
    virtual void EndObject(){};
    virtual void StartObject(std::string key){};
    virtual void EndObject(std::string key){};

    virtual void StartArray(){};
    virtual void EndArray(){};
    virtual void StartArray(std::string key){};
    virtual void EndArray(std::string key){};
};
