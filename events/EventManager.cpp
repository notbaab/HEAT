#include "EventManager.h"
#include "Event.h"
#include "logger/Logger.h"

void EventManager::StaticInit() { sInstance.reset(new EventManager()); }

bool EventManager::AddListener(EventListenerFunction eventDelegate, uint32_t eventType)
{
    // Creates a list if it doesn't exist
    INFO("adding listener of type {}", Event::StringFromId(eventType));
    listeners[eventType].push_back(eventDelegate);
    return true;
}

bool EventManager::RemoveListener(EventListenerFunction eventDelegate, uint32_t eventType) { return false; }

bool EventManager::QueueEvent(std::shared_ptr<Event> event)
{
    eventQueue.push_back(event);
    return true;
}

bool EventManager::AbortEvent(uint32_t eventType) { return false; }

bool EventManager::TriggerEvent(std::shared_ptr<Event> evt)
{
    uint32_t evtType = evt->GetEventType();
    if (listeners.find(evtType) == listeners.end())
    {
        WARN("no events listeners of type {}", evt->IdentifierToString());
        return false; // No events for this type
    }

    for (auto&& listener : listeners[evtType])
    {
        listener(evt);
    }

    return true;
}

void EventManager::FireEvents(uint32_t currentTime)
{
    for (auto&& evt : eventQueue)
    {
        TriggerEvent(evt);
    }

    eventQueue.clear();
}
