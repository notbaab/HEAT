#pragma once

#include <string>

#include "InputStream.h"
#include "str_utils/base64.h"

#include "logger/Logger.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

class JsonInputMemoryStream : public InputStream
{
  public:
    JsonInputMemoryStream() : newData(false) {}

    template <typename T>
    void t_ReadInt(T& inData, const char* name)
    {
        rapidjson::Value* cu = &currentValueStack.back();
        if (inObject())
        {
            inData = cu->GetObject()[name].GetUint64();
        }
        else if (inArray())
        {
            // In a simple array, read and move the index up
            inData = cu->GetArray()[currentArrayIndex.back()].GetUint64();
            currentArrayIndex.back()++;
        }
        else
        {
            // AHAHA
            ERROR("Can't read {}, current value isn't an object or an array", name);
        }
    }

    void Read(uint64_t& inData, const char* name) override { t_ReadInt(inData, name); }
    void Read(uint32_t& inData, const char* name) override { t_ReadInt(inData, name); }
    void Read(uint16_t& inData, const char* name) override { t_ReadInt(inData, name); }
    void Read(int16_t& inData, const char* name) override { t_ReadInt(inData, name); }
    void Read(uint8_t& inData, const char* name) override { t_ReadInt(inData, name); }
    void Read(int8_t& inData, const char* name) override { t_ReadInt(inData, name); }
    void Read(int& inData, const char* name) override { t_ReadInt(inData, name); }

    void Read(float& inData, const char* name) override {

        rapidjson::Value* cu = &currentValueStack.back();
        if (inObject())
        {
            inData = cu->GetObject()[name].GetFloat();
        }
        else if (inArray())
        {
            // In a simple array, read and move the index up
            inData = cu->GetArray()[currentArrayIndex.back()].GetFloat();
            currentArrayIndex.back()++;
        }
        else
        {
            // AHAHA
            ERROR("Can't read {}, current value isn't an object or an array", name);
        }
    }

    void Read(void* inData, uint32_t byteCount, const char* name) override {
        rapidjson::Value* cu = &currentValueStack.back();
        std::string baseEncodedString;

        if (inObject())
        {
            baseEncodedString = cu->GetObject()[name].GetString();
        }
        else if (inArray())
        {
            // In a simple array, read and move the index up
            baseEncodedString = cu->GetArray()[currentArrayIndex.back()].GetString();
            currentArrayIndex.back()++;
        }
        else
        {
            // AHAHA
            ERROR("Can't read {}, current value isn't an object or an array", name);
            return;
        }

        std::string asBytes = base64_decode(baseEncodedString);
        assert(asBytes.size() <= byteCount);
        memcpy(inData, asBytes.c_str(), byteCount);
    }

    void Read(bool& inData, const char* name) override {
        rapidjson::Value* cu = &currentValueStack.back();
        if (inObject())
        {
            inData = cu->GetObject()[name].GetBool();
        }
        else if (inArray())
        {
            // In a simple array, read and move the index up
            inData = cu->GetArray()[currentArrayIndex.back()].GetBool();
            currentArrayIndex.back()++;
        }
        else
        {
            // AHAHA
            ERROR("Can't read {}, current value isn't an object or an array", name);
        }
    }

    void Read(std::string& inString, const char* name) override{
        rapidjson::Value* cu = &currentValueStack.back();
        if (inObject())
        {
            inString = cu->GetObject()[name].GetString();
        }
        else if (inArray())
        {
            // In a simple array, read and move the index up
            inString = cu->GetArray()[currentArrayIndex.back()].GetString();
            currentArrayIndex.back()++;
        }
        else
        {
            // AHAHA
            ERROR("Can't read {}, current value isn't an object or an array", name);
        }
    };

    void StartObject() override
    {
        // Start of the document
        if (currentValueStack.size() == 0)
        {
            // Why I
            Value v = document.GetObject();
            currentValueStack.emplace_back(kObjectType);
            currentValueStack.back() = v;
            newData = false;
            return;
        }

        if (!inArray())
        {
            ERROR("Not in array, need to use start object with a key, this is going to seg fault now");
        }

        Value value = currentValueStack.back().GetArray()[currentArrayIndex.back()].GetObject();

        currentValueStack.emplace_back(kObjectType);
        currentValueStack.back() = value;
    }

    void EndObject() override
    {
        currentValueStack.pop_back();
        if (inArray())
        {
            currentArrayIndex.back()++;
        }
    }
    void StartObject(std::string key) override
    {
        if (inArray())
        {
            ERROR("Can't start key value pair in an array");
            return;
        }

        Value value = currentValueStack.back().GetObject()[key.c_str()].GetObject();

        currentValueStack.emplace_back(kObjectType);
        currentValueStack.back() = value;
    }

    void EndObject(std::string key) override { EndObject(); }

    void StartArray() override
    {
        // Start of the document
        if (currentValueStack.size() == 0)
        {
            Value v = document.GetObject();
            currentValueStack.emplace_back(kArrayType);
            currentValueStack.back() = v;
            newData = false;
            return;
        }

        if (!inArray())
        {
            ERROR("Can't start no keyed array while not in array or at the start of a document");
        }

        Value value = currentValueStack.back().GetArray()[currentArrayIndex.back()].GetArray();

        currentValueStack.emplace_back(kArrayType);
        currentValueStack.back() = value;
        // needs to be for all array that could exist
        currentArrayIndex.emplace_back(0);
    }

    void EndArray() override
    {
        if (!inArray())
        {
            ERROR("Ending array but we aren't in one");
        }
        currentValueStack.pop_back();
        currentArrayIndex.pop_back();
    }

    void StartArray(std::string key) override
    {
        Value value = currentValueStack.back().GetObject()[key.c_str()].GetArray();

        currentValueStack.emplace_back(kArrayType);
        currentValueStack.back() = value;
        // needs to be for all array that could exist
        currentArrayIndex.emplace_back(0);
    }

    void EndArray(std::string key) override
    {
        if (!inArray())
        {
            ERROR("Ending array but we aren't in one");
        }
        currentValueStack.pop_back();
        currentArrayIndex.pop_back();
    }

     bool HasMoreData() override {
        return !currentValueStack.empty() || newData;
    }

    void SetInputBuffer(std::shared_ptr<std::vector<uint8_t>> buf) override
    {
        // Huge problem if not null terminated. Terminate just in case
        if (buf->back() != 0)
        {
            buf->emplace_back(0);
        }

        document.Parse(reinterpret_cast<const char*>(buf->data()));
        currentValueStack.clear();
        currentArrayIndex.clear();

        newData = true;
    };

  private:
    rapidjson::Document document;

    std::vector<rapidjson::Value> currentValueStack;
    std::vector<uint32_t> currentArrayIndex;
    bool newData;

    bool inArray()
    {
        rapidjson::Value* cu = &currentValueStack.back();
        return cu->IsArray();
    }

    bool inObject()
    {
        rapidjson::Value* cu = &currentValueStack.back();
        return cu->IsObject();
    }
};
