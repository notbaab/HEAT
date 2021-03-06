#include <vector>

#include "str_utils.h"

std::unique_ptr<std::vector<std::string>> SplitStr(std::string str, std::string delimiter)
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
