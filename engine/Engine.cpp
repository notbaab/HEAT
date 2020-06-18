#include "Engine.h"
#include "iostream"
#include <thread>

Engine::Engine(IntializerFunc initFunc, TickFunc tickFunc) : Engine(initFunc, tickFunc, []() {}) {}

Engine::Engine(IntializerFunc initFunc, TickFunc tickFunc, CleanUpFunc cleanUpFunc)
    : running(false), initFunc(initFunc), tickFunc(tickFunc), cleanUpFunc(cleanUpFunc), currentFrame(0),
      ticksPerSecond(50), currentTime(0)
{
    initFunc();
    frameTimeInMs = 1000 / ticksPerSecond;
}

void Engine::Run()
{
    running = true;
    while (running)
    {
        auto begin = std::chrono::steady_clock::now();
        running = tickFunc(this->currentTime);
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
        std::this_thread::sleep_for(std::chrono::milliseconds(frameTimeInMs) - duration);
        this->currentFrame++;
        this->currentTime += frameTimeInMs;
    }
}

void Engine::Stop() { running = false; }
// sets the amount of time to run the server at
void Engine::SetTicksPerSecond(uint32_t tps)
{
    // TODO: Add a lock here so the frame time is measure correctly.
    this->ticksPerSecond = tps;
    frameTimeInMs = 1000 / tps;
}
// void Engine::SetSimulationTime(uint32_t simTime) {}
