#include "game/SingleAnimations.h"
#include "game/game.h"
#include "tileCamera2D.h"
#include "config.h"
#include "resource_manager.h"
#include <glm/glm.hpp>
#include <fstream>

// **********************
// JSON deserialization
// **********************
void to_json(nlohmann::json& json, const AnimationInfo& info) {}
void from_json(const nlohmann::json& json, AnimationInfo& info)
{
    json.at("offset").at(0).get_to(info.vOffset.x);
    json.at("offset").at(1).get_to(info.vOffset.y);
    json.at("size").at(0).get_to(info.vSize.x);
    json.at("size").at(1).get_to(info.vSize.y);
    json.at("tile_size").at(0).get_to(info.vTileSize.x);
    json.at("tile_size").at(1).get_to(info.vTileSize.y);
    json.at("duration").get_to(info.nDuration);
}
void to_json(nlohmann::json& json, const std::unordered_map<std::string, AnimationInfo>& infos) {}
void from_json(const nlohmann::json& json, std::unordered_map<std::string, AnimationInfo>& infos)
{
    for (auto it = json.begin(); it != json.end(); ++it)
    {
        from_json(it.value(), infos[it.key()]);
    }
}


// ***************
// SpriteAnimation
// ***************
SpriteAnimation::SpriteAnimation(AnimationInfo* info, glm::vec2 position, glm::vec2 size, bool tile_space, bool one_time)
        : Info(info)
        , vPosition(position)
        , vSize(size)
        , bTileSpace(tile_space)
        , bOneTime(one_time)
{
    init();
}
void SpriteAnimation::Render(SpriteRenderer* pSpriteRenderer, Texture2D& tex)
{
    if (bEnded)
        return;
    glm::vec4& frame = GetFrame();
    if (bTileSpace)
    {
        pSpriteRenderer->DrawPartialSprite(
            tex,
            glm::vec2(frame.x, frame.y),
            glm::vec2(frame.z, frame.w),
            TileCamera2D::GetScreenPosition(vPosition),
            vSize * Game::TileSize * TileCamera2D::GetScale()
        );
    }
    else {
        pSpriteRenderer->DrawPartialSprite(
            tex,
            glm::vec2(frame.x, frame.y),
            glm::vec2(frame.z, frame.w),
            vPosition,
            vSize
        );
    }
}
void SpriteAnimation::Update(float dt)
{
    if (bOneTime && bEnded)
        return;

    fCurrentLife -= dt;
    if (fCurrentLife <= 0.0f)
    {
        nCurrentFrame = (nCurrentFrame + 1) % int(frames.size());
        fCurrentLife = fTotalLife;
        if (bOneTime && nCurrentFrame == 0)
            bEnded = true;
    }
}
void SpriteAnimation::init()
{
    int frame_count = int(Info->vSize.x) / int(Info->vTileSize.x);

    frames.clear();
    frames.reserve(frame_count);
    for (int i = 0 ; i < frame_count; i++)
        frames.push_back(glm::vec4(
            Info->vOffset + glm::vec2(Info->vTileSize.x * i, 0.0f),
            Info->vTileSize
        ));
    
    fTotalLife = float(Info->nDuration) / 1000.0f;   // milliseconds -> seconds
    fCurrentLife = fTotalLife;
    nCurrentFrame = 0;
    bEnded = false;
}

// ******************
// SingleAnimations
// ******************

SingleAnimations::SingleAnimations(std::string properties_path, std::string name) 
    : resource_group_name(name)
    , PropertiesPath(properties_path)
{
    load_infos((properties_path).c_str());
}
SingleAnimations::~SingleAnimations()
{
    for (auto it = OnGoingAnimations.begin(); it != OnGoingAnimations.end(); ++it)
    {
        it->second(it->first);
        delete it->first;
    }
    ResourceManager::DeleteGroup(resource_group_name);
}
void SingleAnimations::Play(std::string animation_name, std::string custom_name, glm::vec2 position, glm::vec2 size, bool tile_space, bool one_time, std::function<void(SpriteAnimation*)> onEnd)
{
    SpriteAnimation* spr = new SpriteAnimation(&Infos[animation_name], position, size, tile_space, one_time);
    spr->sCustomName = custom_name;
    OnGoingAnimations.push_back(std::make_pair(spr, onEnd));
}
void SingleAnimations::Render(SpriteRenderer* pSpriteRenderer)
{
    for (auto& anim : OnGoingAnimations)
        anim.first->Render(pSpriteRenderer, SpriteSheet);
}
void SingleAnimations::Update(float dt)
{
    std::vector<SpriteAnimation*> to_be_removed;
    for (auto& anim : OnGoingAnimations)
    {
        anim.first->Update(dt);
        if (anim.first->bEnded)
        {
            anim.second(anim.first);
            to_be_removed.push_back(anim.first);
        }
    }
    for (size_t i = 0; i < to_be_removed.size(); i++)
    {
        OnGoingAnimations.remove_if([&](auto& anim) { return anim.first == to_be_removed[i]; });
        delete to_be_removed[i];
    }
}
void SingleAnimations::load_infos(const char* path)
{
    std::ifstream ifs(path);
    nlohmann::json j;
    ifs >> j;
    ifs.close();

    std::string sSpritePath = j.at("spritesheet").get<std::string>();
    SpriteSheet = ResourceManager::LoadTexture((ASSETS_DIR + sSpritePath).c_str(), true, sSpritePath, resource_group_name);
    SpriteSheet.SetMagFilter(GL_NEAREST).SetMinFilter(GL_NEAREST).UpdateParameters();

    Infos = j.at("animations").get<std::unordered_map<std::string, AnimationInfo>>();
}