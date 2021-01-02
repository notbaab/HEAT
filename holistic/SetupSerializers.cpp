#include "SetupSerializers.h"

#include "events/CreatePlayerOwnedObject.h"
#include "events/EventManager.h"
#include "events/EventRouter.h"
#include "events/LoggedIn.h"
#include "events/PhysicsComponentUpdate.h"
#include "events/PlayerInputEvent.h"
#include "events/RemoveClientOwnedGameObjectsEvent.h"
#include "events/RemoveGameObjectEvent.h"
#include "messages/ClientConnectionChallengeMessage.h"
#include "messages/ClientConnectionChallengeResponseMessage.h"
#include "messages/ClientConnectionRequestMessage.h"
#include "messages/ClientLoginMessage.h"
#include "messages/ClientLoginResponse.h"
#include "messages/ClientWelcomeMessage.h"
#include "messages/LogoutMessage.h"
#include "messages/PlayerMessage.h"
#include "packets/AuthenticatedPacket.h"
#include "packets/MessageSerializer.h"
#include "packets/PacketSerializer.h"
#include "packets/UnauthenticatedPacket.h"

namespace holistic
{
void SetupPacketSerializer(std::shared_ptr<PacketSerializer> packetSerializer)
{

    AddPacketCtor(packetSerializer, ReliableOrderedPacket);
    AddPacketCtor(packetSerializer, UnauthenticatedPacket);
    AddPacketCtor(packetSerializer, AuthenticatedPacket);
}

void SetupMessageSerializer(std::shared_ptr<MessageSerializer> messageSerializer)
{

    AddMessageCtor(messageSerializer, PlayerMessage);
    AddMessageCtor(messageSerializer, ClientConnectionChallengeMessage);
    AddMessageCtor(messageSerializer, ClientLoginMessage);
    AddMessageCtor(messageSerializer, ClientLoginResponse);

    // Event constructors. Events are also messages
    AddMessageCtor(messageSerializer, CreatePlayerOwnedObject);
    AddMessageCtor(messageSerializer, PhysicsComponentUpdate);
    AddMessageCtor(messageSerializer, PlayerInputEvent);
    AddMessageCtor(messageSerializer, RemoveGameObjectEvent);
    AddMessageCtor(messageSerializer, RemoveClientOwnedGameObjectsEvent);
}

} // namespace holistic
