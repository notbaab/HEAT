#include "TextureManager.h"
#include "GraphicsDriver.h"
#include "Texture.h"
#include <SDL_image.h>
#include <string>

std::unique_ptr<TextureManager> TextureManager::sInstance;

void TextureManager::StaticInit() { sInstance.reset(new TextureManager()); }

TextureManager::TextureManager() {}

TexturePtr TextureManager::GetTexture(const std::string& inTextureName) { return mNameToTextureMap[inTextureName]; }

bool TextureManager::CacheTexture(std::string inTextureName, const char* inFileName)
{
    SDL_Texture* texture = IMG_LoadTexture(GraphicsDriver::sInstance->GetRenderer(), inFileName);

    if (texture == nullptr)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load texture: %s", inFileName);
        return false;
    }

    int w, h;
    SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);

    // Set the blend mode up so we can apply our colors
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    TexturePtr newTexture(new Texture(w, h, texture));

    mNameToTextureMap[inTextureName] = newTexture;

    return true;
}