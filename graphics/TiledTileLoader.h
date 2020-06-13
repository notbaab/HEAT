#pragma once
#include <memory>
#include <string>

class TiledAnimatedSpriteSheetData;

// PIMP Does this need to be a class or just a static load function?
class TiledTileLoader
{
  public:
    TiledTileLoader(){};
    std::unique_ptr<TiledAnimatedSpriteSheetData> LoadAnimationSheetInfo(std::string infoFile);
};
