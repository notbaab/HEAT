#pragma once

#include <cstdint>
#include <memory>

#include "events/PhysicsComponentUpdate.h"
#include "math/Vector3.h"

#define CLASS_IDENTIFICATION(inCode)                                                                                   \
    virtual uint32_t GetClassId() const override { return inCode; }

class EventManager;
// Simple game object with a physical location
class SimpleGameObject
{
  public:
    virtual uint32_t GetClassId() const = 0;
    virtual void SetupListeners(){};
    virtual ~SimpleGameObject(){};
    // SimpleGameObject() : rotation(0) { centerLocation = Vector3(23, 23, 0); };

    // Vector3 GetLocation() {
    //     return centerLocation;
    // }
    void SetRotation(uint16_t degress) { rotation = degress; }
    uint16_t GetRotation() { return rotation; }

    // Simple update function called every tick
    virtual void Update() = 0;

    // TODO: Remove it cause it overlaps with predicted state
    uint16_t rotation;
    Vector3 centerLocation;

    // Game objects might not have a physics componenet
    virtual void HandleStateMessage(std::shared_ptr<PhysicsComponentUpdate> stateEvent) = 0;

    uint32_t GetWorldId() { return worldId; }
    void SetWorldId(uint32_t worldId) { this->worldId = worldId; }

    uint32_t clientOwnerId;

  private:
    // unique id in the world
    uint32_t worldId;
};

typedef std::shared_ptr<SimpleGameObject> GameObjectPtr;
