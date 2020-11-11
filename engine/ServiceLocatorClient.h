#pragma once

#include "holistic/HNetworkManagerClient.h"

class ServiceLocatorClient
{
  public:
    static holistic::HNetworkManagerClient* GetNetworkManagerClient() { return networkService; }
    static void Provide(holistic::HNetworkManagerClient* service) { networkService = service; }

  private:
    inline static holistic::HNetworkManagerClient* networkService;
};
