#pragma once

#include "packets/Message.h"

class PlayerMessage : public Message
{
  public:
    CLASS_IDENTIFIER(PlayerMessage, 'PLAY');

    SERIALIZER;

    enum ReplicationState
    {
        PRS_PID = 1 << 0,  // Player Id
        PRS_POSI = 1 << 1, // Player position and rotation
        ALL_STATE = PRS_POSI | PRS_PID,
    };

    PlayerMessage(){};
    PlayerMessage(ReplicationState state, float xVel, float yVel, float xLoc, float yLoc)
        : state(state), xVel(xVel), yVel(yVel), xLoc(xLoc), yLoc(yLoc){};

    ReplicationState state;

    bool hasPosition, hasId;
    float xVel, yVel, xLoc, yLoc;

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        stream.serialize(id);
        stream.serialize(state);
        hasId = state & ReplicationState::PRS_PID;
        hasPosition = state & ReplicationState::PRS_POSI;

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

        return true;
    }
};
