#pragma once

#include <functional>
#include <memory>

using IntializerFunc = std::function<void()>;
using TickFunc = std::function<bool(uint32_t)>;
using CleanUpFunc = std::function<void()>;

// Very simple class that calls the InitializerFunc at startup
// TickFunc ever frame and TeardownFunc in the stop function
class Engine
{
  public:
    Engine(IntializerFunc initFunc, TickFunc tickFunc);
    Engine(IntializerFunc initFunc, TickFunc tickFunc, CleanUpFunc cleanUpFunc);
    // static uint64_t GetCurrentFrame() { return currentFrame; };

    void Run();
    void Stop();
    void SetTicksPerSecond(uint32_t tps);
    void SetCurrentTime(uint32_t time);

  protected:
    bool running;
    IntializerFunc initFunc;
    TickFunc tickFunc;
    CleanUpFunc cleanUpFunc;

    uint32_t currentFrame;
    uint32_t currentTime;
    uint32_t ticksPerSecond;
    uint32_t frameTimeInMs;
};
