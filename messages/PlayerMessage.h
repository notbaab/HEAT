#pragma once

#include "packets/Message.h"
#include <type_traits>

class PlayerMessage : public Message
{
  public:
    CLASS_IDENTIFIER(PlayerMessage, 'PLAY');

    SERIALIZER;

    // PIMP: Enums and serialization are strange
    enum ReplicationState : uint8_t
    {
        PRS_PID = 1 << 0,  // Player Id
        PRS_POSI = 1 << 1, // Player position and rotation
        ALL_STATE = PRS_POSI | PRS_PID,
    };

    PlayerMessage(){};
    PlayerMessage(ReplicationState state, float xVel, float yVel, float xLoc, float yLoc)
        : state(state), xVel(xVel), yVel(yVel), xLoc(xLoc), yLoc(yLoc)
    {
        stateAsInt = static_cast<uint8_t>(state);
    };

    ReplicationState state;

    bool hasPosition, hasId;
    float xVel, yVel, xLoc, yLoc;

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(id);
        stream.serialize(stateAsInt);
        hasId = stateAsInt & ReplicationState::PRS_PID;
        hasPosition = stateAsInt & ReplicationState::PRS_POSI;

        if (hasId)
        {
            stream.serialize(id);
        }

        if (hasPosition)
        {
            stream.serialize(xVel);
            stream.serialize(yVel);
            stream.serialize(xLoc);
            stream.serialize(yLoc);
        }

        state = static_cast<ReplicationState>(stateAsInt);

        return true;
    }

  private:
    uint8_t stateAsInt;
};
