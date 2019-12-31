#pragma once
#include "packets/Message.h"
#include <unordered_map>

class ClientLoginResponse : public Message
{
  public:
    CLASS_IDENTIFIER(ClientLoginResponse, 'LOGR');
    SERIALIZER;

    template <typename Stream>
    bool Serialize(Stream& stream)
    {
        // The id assigned to the client that logged in.
        stream.serialize(clientId);
        stream.serialize(sizeOfClientMap);

        // Redundant if we are writing. We could check if we are writing to avoid
        // this but it's not as clean looking
        currentClients.resize(sizeOfClientMap);
        for (uint16_t i = 0; i < sizeOfClientMap; ++i)
        {
            stream.serialize(std::get<0>(currentClients[i]));
            stream.serialize(std::get<1>(currentClients[i]));
        }

        return true;
    }

    ClientLoginResponse(){};
    ClientLoginResponse(uint32_t clientId,
                        std::vector<std::tuple<uint32_t, std::string>> currentClients)
        : clientId(clientId), currentClients(currentClients)
    {
        this->sizeOfClientMap = currentClients.size();
    };

    uint32_t clientId;
    // vector of clients as a uid, username pair
    std::vector<std::tuple<uint32_t, std::string>> currentClients;
    // need to explicitly set this and read it so we know how many to read.
    // Writing we can use the actual size of the current clients we get passed in
    uint16_t sizeOfClientMap;
};
