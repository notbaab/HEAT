#pragma once

#include <SDL.h>
#include <memory>
#include <stdint.h>
#include <stdio.h>

class Texture
{
  public:
    Texture(uint32_t inWidth, uint32_t inHeight, SDL_Texture* inTexture)
        : mWidth(inWidth), mHeight(inHeight), mTexture(inTexture){};
    ~Texture() { SDL_DestroyTexture(mTexture); };

    uint32_t GetWidth() const { return mWidth; }
    uint32_t GetHeight() const { return mHeight; }
    SDL_Texture* GetData() const { return mTexture; }

  private:
    uint32_t mWidth;
    uint32_t mHeight;
    SDL_Texture* mTexture;
};

typedef std::shared_ptr<Texture> TexturePtr;
