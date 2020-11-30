#pragma once
#include "World.h"

namespace gameobjects
{

class WorldClient : public World
{
  public:
    static void StaticInit();
    static std::unique_ptr<WorldClient> sInstance;

    void OnStateUpdateMessage(std::shared_ptr<Event> objectStateEvent);
    virtual void OnRemoveClientOwnedObjects(std::shared_ptr<Event> removeClientObject) override;

  private:
    WorldClient();
};

} // namespace gameobjects
