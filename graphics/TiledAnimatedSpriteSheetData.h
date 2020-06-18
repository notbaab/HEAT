#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// where in the sheet to draw it. Probably bigger than the bounds of the tile
struct DrawRect
{
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
};

// Global sheet data. How big is it, tile info etc
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

enum class AnimationOrientation
{
    NONE,
    UP,
    DOWN,
    LEFT,
    RIGHT
};

AnimationOrientation AnimationOrientationFromString(std::string);

struct SpriteAnimationData
{
    // TODO: Maybe an enum with a mapping so we don't get crazy with types of animations?
    std::string type;
    // TODO: Also maybe an enum is more appropriate
    std::string name;
    // TODO: orientation is weird as a string. NOT A FAN
    AnimationOrientation orientation;
    std::vector<SpriteAnimationFrameData> animations;

    uint16_t NumAnimations() { return animations.size(); }
};

class TiledAnimatedSpriteSheetData
{
  public:
    TiledAnimatedSpriteSheetData(std::shared_ptr<TiledSheetData> sheetData)
        : baseSheetData(sheetData), name(sheetData->name){};

    bool PushAnimationData(std::string name, SpriteAnimationData frameData)
    {
        // Assuming the first animation added is the idle animation. Could
        // add a whole thing to set the first animation but just
        // making it the top of the sheet is simple and makes sense.
        animations[name] = frameData;

        return true;
    };

    DrawRect ConvertIdToRect(uint32_t tileId) { return baseSheetData->ConvertIdToRect(tileId); }

    std::unordered_map<std::string, SpriteAnimationData> animations;
    std::shared_ptr<TiledSheetData> baseSheetData;
    std::string name;
    std::shared_ptr<SpriteAnimationData> firstAnimation;
};
