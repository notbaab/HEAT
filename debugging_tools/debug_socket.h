#include <functional>
#include <string>
#include <vector>

using ReadCallback = std::function<std::string(uint8_t* data, size_t size)>;
bool SpawnSocket(const char* socket_path, ReadCallback readCallback);
void StopSocketThreads();
