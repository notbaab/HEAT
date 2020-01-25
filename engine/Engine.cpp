#include "Engine.h"
#include "iostream"
#include <thread>

Engine::Engine(IntializerFunc initFunc, TickFunc tickFunc)
    : running(false), initFunc(initFunc), tickFunc(tickFunc), currentFrame(0)
{
    initFunc();
}

void Engine::Run()
{
    running = true;
    while (running)
    {
        auto begin = std::chrono::steady_clock::now();
        running = tickFunc(this->currentFrame * 30);
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
        std::this_thread::sleep_for(std::chrono::milliseconds(10) - duration);
        this->currentFrame++;
    }
}

void Engine::Stop() { running = false; }
