#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace str_utils
{
std::unique_ptr<std::vector<std::string>> SplitString(std::string str, std::string delimiter)
{
    auto strings = std::make_unique<std::vector<std::string>>();

    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    size_t strSize = delimiter.size();

    while ((pos = str.find(delimiter, prev)) != std::string::npos)
    {
        auto splitStr = str.substr(prev, pos - prev);
        strings->emplace_back(splitStr);
        prev = pos + strSize;
    }

    return strings;
}

template <typename... Args>
std::string string_format(const std::string& format, Args... args)
{
    size_t size = snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
    if (size <= 0)
    {
        throw std::runtime_error("Error during formatting.");
    }
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}
} // namespace str_utils
