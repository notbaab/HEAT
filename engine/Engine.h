#pragma once

#include <functional>
#include <memory>

using IntializerFunc = std::function<void()>;
using TickFunc = std::function<bool(uint32_t)>;

// Very simple class that calls the InitializerFunc at startup
// TickFunc ever frame and TeardownFunc in the stop function
class Engine
{
  public:
    Engine(IntializerFunc initFunc, TickFunc tickFunc);
    // static uint64_t GetCurrentFrame() { return currentFrame; };

    void Run();
    void Stop();

  protected:
    bool running;
    IntializerFunc initFunc;
    TickFunc tickFunc;
    uint32_t currentFrame;
};
