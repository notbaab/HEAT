#pragma once
#include <iostream>
#include <memory>
#include <vector>

using std::cout;
using std::endl;

void printStream(uint32_t bufferSize, const char* streamBuffer);
void printStream(uint32_t bufferSize, const std::shared_ptr<std::vector<uint8_t>> streamBuffer);