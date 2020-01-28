#pragma once
#include <unordered_map>

#include "logger/Logger.h"

class Event;

// Used when we have an event that needs to be routed to specific instances of
// class T. Class T must have a unique id called worldId.
template <class EvtClass>
class EventRouter
{
  public:
    static inline std::unique_ptr<EventRouter<EvtClass>> sInstance;
    static void StaticInit() { sInstance.reset(new EventRouter<EvtClass>()); }

    bool AddListener(uint32_t id, std::function<void(std::shared_ptr<EvtClass>)> funcCall)
    {
        if (inListenerMap(id))
        {
            return false;
        }

        listeners[id] = funcCall;
        return true;
    }

    bool RouteEvent(std::shared_ptr<Event> evt)
    {
        auto castedEvt = std::static_pointer_cast<EvtClass>(evt);

        TRACE("Routing {}", castedEvt->worldId);
        if (!inListenerMap(castedEvt->worldId))
        {
            TRACE("Did not route");
            return false;
        }

        TRACE("Calling");
        listeners[castedEvt->worldId](castedEvt);
        return true;
    }

  private:
    EventRouter<EvtClass>(){};
    std::unordered_map<uint32_t, std::function<void(std::shared_ptr<EvtClass>)>> listeners;
    bool inline inListenerMap(uint32_t id) { return listeners.find(id) != listeners.end(); }
};
