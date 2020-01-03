#pragma once

#include "math/Vector3.h"
#include <cstdint>
#include <memory>

#define CLASS_IDENTIFICATION(inCode)                                                               \
    virtual uint32_t GetClassId() const override { return inCode; }

// Simple game object with a physical location
class SimpleGameObject
{
  public:
    virtual uint32_t GetClassId() const = 0;
    virtual ~SimpleGameObject(){};
    // SimpleGameObject() : rotation(0) { centerLocation = Vector3(23, 23, 0); };

    Vector3 GetLocation() { return centerLocation; }
    // std::string getSpriteSheet() { return spriteSheetData.sheetLoc; }
    void SetRotation(uint16_t degress) { rotation = degress; }
    uint16_t GetRotation() { return rotation; }

    // Simple update function called every tick
    virtual void Update() = 0;

    uint16_t rotation;
    Vector3 centerLocation;
};

typedef std::shared_ptr<SimpleGameObject> GameObjectPtr;
