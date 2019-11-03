#pragma once

#include "Texture.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class TextureManager
{
  public:
    static void StaticInit();

    static std::unique_ptr<TextureManager> sInstance;

    TexturePtr GetTexture(const std::string& inTextureName);

  private:
    TextureManager();

    bool CacheTexture(std::string inName, const char* inFileName);

    // MAKE A VECTOR!!!
    std::unordered_map<std::string, TexturePtr> mNameToTextureMap;
};
