#pragma once

#include <functional>
#include <list>
#include <memory>
#include <unordered_map>

class Event;
using EventListenerFunction = std::function<void(std::shared_ptr<Event>)>;

// PIMP you should prefer lambdas instead of bind
#define CREATE_DELEGATE(func, obj) std::bind(func, std::ref(obj), std::placeholders::_1)

// Only works with static objects. Probably the main use case we have so doing it this way from now
// on
#define CREATE_DELEGATE_LAMBDA(func) [](std::shared_ptr<Event> evt) { func(evt); }

#define CREATE_DELEGATE_LAMBDA_CAPTURE(obj, func) [&obj](std::shared_ptr<Event> evt) { obj->func(evt); }

class EventManager
{
  public:
    static inline std::unique_ptr<EventManager> sInstance;
    static void StaticInit();

    bool AddListener(EventListenerFunction eventDelegate, uint32_t eventType);
    bool RemoveListener(EventListenerFunction eventDelegate, uint32_t eventType);

    bool QueueEvent(std::shared_ptr<Event> event);
    bool AbortEvent(uint32_t eventType);
    bool TriggerEvent(std::shared_ptr<Event> event);

    // Called every tick to fire any events in the queue that are less then the
    // current time
    void FireEvents(uint32_t currentTime);

  private:
    EventManager(){};
    std::unordered_map<uint32_t, std::list<EventListenerFunction>> listeners;
    std::list<std::shared_ptr<Event>> eventQueue;
};
