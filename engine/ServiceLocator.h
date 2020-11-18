
#include "holistic/HNetworkManager.h"

class ServiceLocator
{
  public:
    static holistic::HNetworkManager* GetNetworkManager() { return networkService; }
    static void Provide(holistic::HNetworkManager* service) { networkService = service; }

  private:
    inline static holistic::HNetworkManager* networkService;
};
