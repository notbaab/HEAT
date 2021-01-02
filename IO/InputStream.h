#pragma once

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <vector>

class InputStream
{
  public:
    InputStream(){};
    virtual ~InputStream() = default;

    virtual void Read(uint64_t& outData, const char* name) = 0;
    virtual void Read(uint32_t& outData, const char* name) = 0;
    virtual void Read(int& outData, const char* name) = 0;
    virtual void Read(float& outData, const char* name) = 0;

    // Read some raw bytes out of the underlying buffer. I don't think I actually want this
    virtual void Read(void* outData, uint32_t byteCount, const char* name) = 0;

    virtual void Read(uint16_t& outData, const char* name) = 0;
    virtual void Read(int16_t& outData, const char* name) = 0;

    virtual void Read(uint8_t& outData, const char* name) = 0;
    virtual void Read(int8_t& outData, const char* name) = 0;
    virtual void Read(bool& outData, const char* name) = 0;

    virtual void Read(std::string& inString, const char* name) = 0;

    virtual void SetInputBuffer(std::shared_ptr<std::vector<uint8_t>> buf) = 0;
    virtual bool HasMoreData() = 0;

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
