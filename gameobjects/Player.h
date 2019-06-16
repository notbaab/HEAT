#pragma once

#include "SimpleGameObject.h"

#define PLAYER_ID 0x13129328

class Player : public SimpleGameObject
{
  public:
    CLASS_IDENTIFICATION(PLAYER_ID)
    static std::unique_ptr<SimpleGameObject> StaticCreate()
    {
        return std::unique_ptr<SimpleGameObject>(new Player());
    }

    Player(){};
};
