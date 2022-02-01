#pragma once
#include "texture.h"
#include "sprite_renderer.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>
#include <list>
#include <functional>
#include <nlohmann/json.hpp>

struct AnimationInfo
{
    // Offset, size and tile_size in pixels.
    glm::vec2 vOffset;
    glm::vec2 vSize;
    glm::vec2 vTileSize;
    // Duration in milliseconds.
    int nDuration;
};
class SpriteAnimation
{
public:
    AnimationInfo* Info = nullptr;
    bool bEnded = true;
    bool bOneTime = true;
    bool bTileSpace = false;
    glm::vec2 vPosition = glm::vec2(0.0f, 0.0f);
    glm::vec2 vSize = glm::vec2(0.0f, 0.0f);
    std::string sCustomName = "";

    SpriteAnimation() = default;
    SpriteAnimation(AnimationInfo* info, glm::vec2 position, glm::vec2 size, bool tile_space, bool one_time);

    void Render(SpriteRenderer* pSpriteRenderer, Texture2D& tex);
    void Update(float dt);
    glm::vec4& GetFrame() { return frames[nCurrentFrame]; }

protected:
    float fTotalLife = 0.0f;
    float fCurrentLife = 0.0f;
    std::vector<glm::vec4> frames;
    int nCurrentFrame = 0;

    void init();
};

class SingleAnimations
{
public:
    Texture2D SpriteSheet;
    std::unordered_map<std::string, AnimationInfo> Infos;
    std::list<std::pair<SpriteAnimation*, std::function<void(SpriteAnimation*)>>> OnGoingAnimations;
    std::string PropertiesPath = "";

    SingleAnimations(std::string properties_path, std::string name);
    ~SingleAnimations();

    void Play(std::string animation_name, std::string custom_name, glm::vec2 position, glm::vec2 size, bool tile_space, bool one_time = true, std::function<void(SpriteAnimation*)> onEnd = [](SpriteAnimation* s){});
    void Render(SpriteRenderer* pSpriteRenderer);
    void Update(float dt);
protected:
    std::string resource_group_name;

    void load_infos(const char* path);
};

void to_json(nlohmann::json& json, const AnimationInfo& info);
void from_json(const nlohmann::json& json, AnimationInfo& info);
void to_json(nlohmann::json& json, const std::unordered_map<std::string, AnimationInfo>& infos);
void from_json(const nlohmann::json& json, std::unordered_map<std::string, AnimationInfo>& infos);

// class SingleAnimationsManager
// {
// public:
//     std::unordered_map<std::string, std::vector<

//     void LoadAnimations(std::string properties_path, std::string name);
//     void DeleteAnimations(std::string name);

//     void Play(std::string animation_name, glm::vec2 position, glm::vec2 size, bool tile_space, std::function<void (std::string)> onEnd);
//     void Update(float dt);
//     void Render(SpriteRenderer* pSpriteRenderer);
// };