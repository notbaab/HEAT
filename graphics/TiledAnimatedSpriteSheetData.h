#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "math/Vector3.h"

// where in the sheet to draw it. Probably bigger than the bounds of the tile
struct DrawRect
{
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
};

// Global sheet data. How big
struct TiledSheetData
{
    uint32_t width;
    uint32_t height;
    uint32_t columns;
    uint32_t spacing;
    uint32_t margin;
    uint32_t tileCount;
    uint32_t tileWidth;
    uint32_t tileHeight;

    std::string tiledVersion;
    std::string name;

    DrawRect ConvertIdToRect(uint32_t tileId);
};

// A single frame of animation data. Where to draw it from the sheet with
// the tile id (which can tell use where to draw it) and how long to draw it
// for
struct SpriteAnimationFrameData
{
    uint32_t tileid;

    DrawRect drawRect;
    uint32_t duration;
    // hitbox info here?
};

struct TileData
{
};

struct Properties
{
};

struct SpriteAnimationData
{
    SpriteAnimationData(std::string name, MovementType type, MovementOrientation orientation,
                        std::vector<SpriteAnimationFrameData> animations, bool flipped)
        : type(type), name(name), orientation(orientation), animations(animations), flipped(flipped)
    {
    }

    SpriteAnimationData(const SpriteAnimationData& animData)
        : type(animData.type), name(animData.name), orientation(animData.orientation), animations(animData.animations),
          flipped(animData.flipped)
    {
    }

    // TODO: Maybe an enum with a mapping so we don't get crazy with types of animations?
    MovementType type;
    // TODO: Also maybe an enum is more appropriate
    std::string name;

    MovementOrientation orientation;

    std::vector<SpriteAnimationFrameData> animations;
    bool flipped;

    uint16_t NumAnimations() { return animations.size(); }
};

class TiledAnimatedSpriteSheetData
{
  public:
    TiledAnimatedSpriteSheetData(std::shared_ptr<TiledSheetData> sheetData)
        : baseSheetData(sheetData), name(sheetData->name), firstAnimation(""){};

    bool PushAnimationData(SpriteAnimationData frameData)
    {
        // Assuming the first animation added is the idle animation. Could
        // add a whole thing to set the first animation but just
        // making it the top of the sheet is simple and makes sense.
        if (firstAnimation == "")
        {
            firstAnimation = frameData.name;
        }

        // TODO: copy constructor? Hmmmm.....
        // animations.emplace(frameData.name, SpriteAnimationData(frameData));

        auto shared = std::make_shared<SpriteAnimationData>(frameData);

        animations.emplace(frameData.name, shared);

        auto search = byType.find(frameData.type);

        // PIMP: Maybe don't copy the vector?
        std::vector<std::shared_ptr<SpriteAnimationData>> byTypeVector;

        if (search == byType.end())
        {
            byType.emplace(frameData.type, std::vector<std::shared_ptr<SpriteAnimationData>>());
        }

        byType[frameData.type].push_back(shared);

        return true;
    };

    std::shared_ptr<SpriteAnimationData> GetAnimation(MovementType type, MovementOrientation orientation)
    {
        auto search = byType.find(type);
        if (search == byType.end())
        {
            return nullptr;
        }

        for (auto& anim : search->second)
        {
            if (anim->orientation == orientation)
            {
                return anim;
            }
        }
        return nullptr;
    }

    DrawRect ConvertIdToRect(uint32_t tileId) { return baseSheetData->ConvertIdToRect(tileId); }

    std::unordered_map<std::string, std::shared_ptr<SpriteAnimationData>> animations;
    std::unordered_map<MovementType, std::vector<std::shared_ptr<SpriteAnimationData>>> byType;
    std::shared_ptr<TiledSheetData> baseSheetData;
    std::string name;
    std::string firstAnimation;
};
