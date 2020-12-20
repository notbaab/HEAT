#pragma once

#include "OutputStream.h"
#include "str_utils/base64.h"

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

class JsonOutputMemoryStream : public OutputStream
{
  public:
    JsonOutputMemoryStream() : writer(buf) {}
    void Write(uint64_t inData, const char* name) override
    {
        writeKey(name);
        writer.Uint64(inData);
    }

    void Write(uint32_t inData, const char* name) override
    {
        writeKey(name);
        writer.Uint(inData);
    }

    void Write(uint16_t inData, const char* name) override
    {
        writeKey(name);
        writer.Uint(inData);
    }

    void Write(int16_t inData, const char* name) override
    {
        writeKey(name);
        writer.Uint(inData);
    }

    void Write(uint8_t inData, const char* name) override
    {
        writeKey(name);
        writer.Uint(inData);
    }

    void Write(int8_t inData, const char* name) override
    {
        writeKey(name);
        writer.Uint(inData);
    }

    void Write(int inData, const char* name) override
    {
        writeKey(name);
        writer.Int(inData);
    }

    void Write(float inData, const char* name) override
    {
        writeKey(name);
        writer.Double(inData);
    }
    void Write(void* inData, uint32_t byteCount, const char* name) override
    {
        writeKey(name);
        auto str = base64_encode(static_cast<unsigned char const*>(inData), byteCount, false);
        writer.String(str.c_str());
    }

    void Write(bool inData, const char* name) override
    {
        writeKey(name);
        writer.Bool(inData);
    }

    void Write(const std::string& inString, const char* name) override
    {
        writeKey(name);
        writer.String(inString.c_str());
    }

    void StartObject() override { writer.StartObject(); }
    void EndObject() override { writer.EndObject(); }
    void StartObject(std::string key) override
    {
        writeKey(key.c_str());
        writer.StartObject();
    }
    void EndObject(std::string key) override { writer.EndObject(); }

    void StartArray() override { writer.StartArray(); }
    void EndArray() override { writer.EndArray(); }
    void StartArray(std::string key) override
    {
        writeKey(key.c_str());
        writer.StartArray();
    }
    void EndArray(std::string key) override { writer.EndArray(); }
    void ResetBuffer() override {}

    const std::shared_ptr<std::vector<uint8_t>> GetSharedBuffer() const override
    {
        return std::shared_ptr<std::vector<uint8_t>>();
    }

    const uint8_t* GetRawBuffer() const override
    {
        const char* output = buf.GetString();
        return (const uint8_t*)(output);
    }
    const uint32_t BytesInBuffer() const override {
        return buf.GetSize();
    }

  private:
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer;

    void writeKey(const char* key)
    {
        if (strlen(key) != 0)
        {
            writer.Key(key);
        }
    }
};
