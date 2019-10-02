#include "Engine.h"
#include "iostream"
#include <thread>

Engine::Engine(IntializerFunc initFunc, TickFunc tickFunc)
    : running(false), initFunc(initFunc), tickFunc(tickFunc)
{
    initFunc();
}

void Engine::Run()
{
    running = true;
    while (running)
    {
        auto begin = std::chrono::steady_clock::now();
        running = tickFunc();
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
        std::this_thread::sleep_for(std::chrono::milliseconds(10) - duration);
    }
}

void Engine::Stop() { running = false; }
