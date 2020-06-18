#include <algorithm>
#include <fstream>

#include "logger/Logger.h"

#include "TiledAnimatedSpriteSheetData.h"
#include "TiledTileLoader.h"

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

// The key we use in the properties object to set an animation name
#define ANIM_NAME_KEY "name"

// The key we use in the properties object for animation orientation
#define ANIM_ORIENTATION_KEY "orientation"

// returns where in the sheet to draw the sprite
DrawRect TiledSheetData::ConvertIdToRect(uint32_t tileId)
{
    assert(tileId < tileCount);
    int column = tileId % columns;
    int row = tileId / columns;

    INFO("ID{} {} {}", tileId, column, row);

    uint32_t spacingColumns = std::max(0, column - 1);
    uint32_t spacingRows = std::max(0, row - 1);

    uint32_t totalXSpacing = spacingColumns * (margin + spacing);
    uint32_t totalYSpacing = spacingRows * (margin + spacing);

    DrawRect rect;
    rect.width = tileWidth;
    rect.height = tileHeight;
    rect.x = totalXSpacing + column * tileWidth;
    rect.y = totalYSpacing + row * tileHeight;

    return rect;
}

static bool HandleAnimationData(rapidjson::Value::Object& tile, TiledAnimatedSpriteSheetData& outSheetData)
{
    assert(tile["id"].IsUint());
    unsigned int id = tile["id"].GetUint();
    if (!tile.HasMember("type"))
    {
        ERROR("Animation for id {} has no type. Not adding", id);
        return false;
    }

    assert(tile["type"].IsString());
    std::string animationType = tile["type"].GetString();

    if (!tile.HasMember("properties"))
    {
        ERROR("No properties value for id {} but animation data exists. Unsure how to "
              "interpret animation. Please set property 'name' with the name of the "
              "animation",
              id);
        return false;
    }

    assert(tile["properties"].IsArray());

    // janky but theses are the properties we care about
    std::string animationName;
    AnimationOrientation orientation = AnimationOrientation::NONE;

    for (auto& p : tile["properties"].GetArray())
    {
        assert(p["name"].IsString());
        auto propertyName = p["name"].GetString();
        if (strncmp(propertyName, ANIM_NAME_KEY, sizeof(*propertyName)) == 0)
        {
            assert(p["value"].IsString());
            animationName = p["value"].GetString();
        }
        else if (strncmp(propertyName, ANIM_ORIENTATION_KEY, sizeof(*propertyName)) == 0)
        {
            assert(p["value"].IsString());
            std::string orientationStr = p["value"].GetString();
            orientation = AnimationOrientationFromString(orientationStr);
        }
    }

    if (animationName.empty())
    {
        ERROR("Animations in tile {} need a name, not adding", id);
        return false;
    }

    assert(tile["animation"].IsArray());

    auto animationTileData = tile["animation"].GetArray();
    std::vector<SpriteAnimationFrameData> animatedTiles;
    animatedTiles.reserve(animationTileData.Size());

    INFO("{} has animation", id);

    for (auto& v : animationTileData)
    {
        assert(v["tileid"].IsUint());
        assert(v["duration"].IsUint());

        SpriteAnimationFrameData frameData;
        frameData.tileid = v["tileid"].GetUint();
        frameData.duration = v["duration"].GetUint();
        frameData.drawRect = outSheetData.ConvertIdToRect(frameData.tileid);

        animatedTiles.push_back(frameData);
    }

    SpriteAnimationData animationData;
    animationData.type = animationType;
    animationData.name = animationName;
    animationData.orientation = orientation;
    animationData.animations = animatedTiles;

    outSheetData.PushAnimationData(animationName, animationData);

    return true;
}

// if created with tiled these will all pass but always good to be safe
static void BaseSanityCheck(rapidjson::Value::Object& sheet)
{
    assert(sheet["imagewidth"].IsUint());
    assert(sheet["imageheight"].IsUint());
    assert(sheet["columns"].IsUint());
    assert(sheet["spacing"].IsUint());
    assert(sheet["margin"].IsUint());
    assert(sheet["name"].IsString());
    assert(sheet["tilecount"].IsUint());
    assert(sheet["tilewidth"].IsUint());
    assert(sheet["tileheight"].IsUint());
    assert(sheet["tiledversion"].IsString());
    assert(sheet["tiles"].IsArray());
}

static void PopulateBaseData(rapidjson::Value::Object& sheetObj, TiledSheetData& outSheet)
{
    outSheet.width = sheetObj["imagewidth"].GetUint();
    outSheet.height = sheetObj["imageheight"].GetUint();
    outSheet.columns = sheetObj["columns"].GetUint();
    outSheet.spacing = sheetObj["spacing"].GetUint();
    outSheet.margin = sheetObj["margin"].GetUint();
    outSheet.tileCount = sheetObj["tilecount"].GetUint();
    outSheet.tileWidth = sheetObj["tilewidth"].GetUint();
    outSheet.tileHeight = sheetObj["tileheight"].GetUint();

    outSheet.name = sheetObj["name"].GetString();
    outSheet.tiledVersion = sheetObj["tiledversion"].GetString();
}

std::unique_ptr<TiledAnimatedSpriteSheetData> TiledTileLoader::LoadAnimationSheetInfo(std::string infoFile)
{
    std::ifstream ifs(infoFile);
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document d;

    d.ParseStream(isw);
    if (d.HasParseError())
    {
        ERROR("Error loading data");
        return nullptr;
    }

    assert(d.IsObject());
    auto obj = d.GetObject();
    BaseSanityCheck(obj);

    std::shared_ptr<TiledSheetData> sheet = std::make_shared<TiledSheetData>();
    PopulateBaseData(obj, *sheet);

    auto animatedSpriteData = std::make_unique<TiledAnimatedSpriteSheetData>(sheet);

    auto tiles = obj["tiles"].GetArray();

    for (auto& v : tiles)
    {
        assert(v.IsObject());
        auto tile = v.GetObject();

        assert(tile["id"].IsUint());
        unsigned int id = tile["id"].GetUint();

        if (tile.HasMember("animation"))
        {
            HandleAnimationData(tile, *animatedSpriteData);
        }

        if (tile.HasMember("objectgroup)"))
        {
            // TODO: collision boxes?
        }
    }

    for (auto& anim : animatedSpriteData->animations)
    {
        auto spriteData = anim.second;
        INFO("Animation name {}, type {}", spriteData.name, spriteData.type);
    }

    return std::move(animatedSpriteData);
}
