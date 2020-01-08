#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

// Should be a four element vector in the order of x, y, width, height
typedef std::vector<uint32_t> SingleFrameData;

// A vector single frame data designed to be played as an animation
typedef std::vector<SingleFrameData> AnimationFrameData;

// Holds all animations read from a sprite sheet with the animation data
// indexed
// typedef std::vector<AnimationFrameData> SpriteSheetAnimationFrameData;
typedef std::unordered_map<std::string, AnimationFrameData> SpriteSheetAnimationFrameData;

SingleFrameData createSingleFrame(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    SingleFrameData frameInfo = {x, y, width, height};
    return frameInfo;
}

class SpriteSheetData
{
  public:
    SpriteSheetData(std::string sheetLoc, std::string dataLoc) : sheetLoc(sheetLoc)
    {
        std::ifstream ifs(dataLoc);
        rapidjson::IStreamWrapper isw(ifs);
        rapidjson::Document d;

        d.ParseStream(isw);
        if (d.HasParseError())
        {
            std::cout << "ERROR loading data" << std::endl;
            return;
        }

        assert(d.IsObject());

        auto obj = d.GetObject();
        auto staticTextures = obj["static_textures"].GetArray();

        for (auto& v : staticTextures)
        {
            // Single frame since it's just a static image
            SingleFrameData info = createSingleFrame(v["x"].GetInt(), v["y"].GetInt(),
                                                     v["width"].GetInt(), v["height"].GetInt());
            auto name = v["name"].GetString();
            staticTextureMap[name] = info;
        }

        auto animationsData = obj["animations"].GetArray();
        for (auto& v : animationsData)
        {
            auto animation = v.GetObject();
            auto animationName = animation["name"].GetString();
            auto frames = animation["frames"].GetArray();

            // Full animation vector
            AnimationFrameData frameData;

            std::vector<uint32_t> frameInfo(frames.Size());
            for (auto& frame : frames)
            {
                SingleFrameData info =
                    createSingleFrame(frame["x"].GetInt(), frame["y"].GetInt(),
                                      frame["width"].GetInt(), frame["height"].GetInt());
                frameData.push_back(info);
            }

            animations[animationName] = frameData;
        }
    }

    std::string sheetLoc;

    // map from name to single frame data on the give sprite sheet
    std::unordered_map<std::string, SingleFrameData> staticTextureMap;
    std::unordered_map<std::string, AnimationFrameData> animations;

    static void RegisterSpriteSheetData(std::string name, std::string sheetLoc, std::string dataLoc)
    {
        auto sheetData = std::make_shared<SpriteSheetData>(sheetLoc, dataLoc);
        Registry[name] = sheetData;
    }

    static std::shared_ptr<SpriteSheetData> GetSheetData(std::string name)
    {
        if (Registry.find(name) == Registry.end())
        {
            throw std::runtime_error("No data found for specified name");
        }

        return Registry[name];
    }

  private:
    static inline std::unordered_map<std::string, std::shared_ptr<SpriteSheetData>> Registry;
};
