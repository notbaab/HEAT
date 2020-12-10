#include <SDL.h>
#include <SDL_image.h>
#include <fstream>

#include "AssetManager.h"
#include "GraphicsDriver.h"
#include "TiledAnimatedSpriteSheetData.h"
#include "logger/Logger.h"

std::unique_ptr<AssetManager> AssetManager::sInstance;

AssetManager::~AssetManager()
{
    // Clean up yo shit
    for (auto textureEntry : textureRegistry)
    {
        SDL_DestroyTexture(textureEntry.second);
    }
}

// TODO: Single asset map file? Maybe
bool AssetManager::StaticInit()
{
    sInstance.reset(new AssetManager());
    return true;
}

std::shared_ptr<TiledAnimatedSpriteSheetData> AssetManager::GetAnimatedSheetData(std::string name)
{
    auto search = animatedSheetDataRegistry.find(name);

    if (search == animatedSheetDataRegistry.end())
    {
        ERROR("Didn't find sheet {}", name);
        throw std::runtime_error("No sheet data found for specified name");
    }

    return search->second;
}

bool AssetManager::PushAnimatedTiledSheet(std::unique_ptr<TiledAnimatedSpriteSheetData> sheetData,
                                          std::string sheetTextureLoc)
{
    auto name = sheetData->name;
    if (TextureExists(name))
    {
        ERROR("Pushing Animated Tiled Sheet {} twice. Sheet data must have unique names");
        return false;
    }

    if (!CacheTexture(name, sheetTextureLoc))
    {
        return false;
    }

    animatedSheetDataRegistry[name] = std::move(sheetData);

    return true;
}

bool AssetManager::TextureExists(const std::string& inTextureName)
{
    return textureRegistry.find(inTextureName) != textureRegistry.end();
}

SDL_Texture* AssetManager::GetTexture(const std::string& inTextureName)
{
    auto search = textureRegistry.find(inTextureName);

    if (search == textureRegistry.end())
    {
        ERROR("Didn't find texture {}", inTextureName);
        // PIMP Remove runtime errors this low
        throw std::runtime_error("Error getting texture");
    }

    return search->second;
}

bool AssetManager::CacheTexture(std::string inTextureName, std::string inFileName)
{
    return CacheTexture(inTextureName, inFileName.c_str());
}

bool AssetManager::CacheTexture(std::string inTextureName, const char* inFileName)
{
    // PIMP: pass the renderer in the constructor
    INFO("Caching {} with name {}", inFileName, inTextureName);
    SDL_Texture* texture = IMG_LoadTexture(GraphicsDriver::sInstance->GetRenderer(), inFileName);

    if (texture == nullptr)
    {
        ERROR("SDL Couldn't load texture {}. See SDL Error", inFileName);
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load texture: %s", inFileName);
        return false;
    }

    // int w, h;
    // SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);

    // // Set the blend mode up so we can apply our colors
    // SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    // TexturePtr newTexture(new Texture(w, h, texture));

    textureRegistry[inTextureName] = texture;

    return true;
}