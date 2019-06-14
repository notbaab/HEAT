#include "Engine.h"
#include "iostream"

Engine::Engine(IntializerFunc initFunc, TickFunc tickFunc)
    : running(false), initFunc(initFunc), tickFunc(tickFunc)
{
    initFunc();
}

void Engine::Run()
{
    running = true;
    while (tickFunc() && running)
    {
    }
}

void Engine::Stop() { running = false; }
