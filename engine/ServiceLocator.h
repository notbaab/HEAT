#pragma once

#include "holistic/HNetworkManager.h"

class ServiceLocator
{
  public:
    static holistic::HNetworkManager* GetNetworkManager() { return networkService; }

    template <typename T>
    static T GetNetworkManager()
    {
        return static_cast<T>(networkService);
    }

    static void Provide(holistic::HNetworkManager* service) { networkService = service; }

  private:
    inline static holistic::HNetworkManager* networkService;
};
