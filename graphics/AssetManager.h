#pragma once

#include <iostream>
#include <string>

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

#include "TextureManager.h"

class SpriteSheetData;
class SDL_Texture;
class TiledAnimatedSpriteSheetData;

// Take a file with
class AssetManager
{
  public:
    static bool StaticInit(std::string assetMap);

    static std::unique_ptr<AssetManager> sInstance;
    bool LoadAssetMap(std::string assetMap);

    bool PushAnimatedTiledSheet(std::unique_ptr<TiledAnimatedSpriteSheetData> sheetData,
                                std::string sheetLoc);

    SDL_Texture* GetTexture(const std::string& inTextureName);
    bool TextureExists(const std::string& inTextureName);
    bool CacheTexture(std::string inTextureName, const char* inFileName);
    bool CacheTexture(std::string inTextureName, std::string inFileName);

    std::shared_ptr<TiledAnimatedSpriteSheetData> GetAnimatedSheetData(std::string name);

  private:
    AssetManager() {}
    AssetManager(std::string assetMap) { LoadAssetMap(assetMap); }

    std::unordered_map<std::string, std::shared_ptr<TiledAnimatedSpriteSheetData>>
        animatedSheetDataRegistry;

    std::unordered_map<std::string, SDL_Texture*> textureRegistry;
};