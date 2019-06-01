#pragma once

#include "math/Vector3.h"
#include <cstdint>

// Simple drawable game object class
class SimpleGameObject
{
  public:
    SimpleGameObject() : rotation(0) { centerLocation = Vector3(23, 23, 0); };

    Vector3 GetLocation() { return centerLocation; }
    // std::string getSpriteSheet() { return spriteSheetData.sheetLoc; }
    void SetRotation(uint16_t degress) { rotation = degress; }
    uint16_t GetRotation() { return rotation; }

    // Simple update function called every tick
    void update();

    uint16_t rotation;
    Vector3 centerLocation;
};