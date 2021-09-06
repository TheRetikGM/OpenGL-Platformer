#pragma once
#include "texture.h"
#include "game/AnimatedSprite.h"
#include <tuple>
#include <map>
#include <any>
#include "nlohmann/json.hpp"

enum class ParameterType : uint8_t {
		p_int, p_float, p_string, p_bool
};

class AnimationManager
{
public:
    std::string Name;
    std::map<std::string, std::map<std::string, AnimatedSprite>> Animations;
    std::map<std::string, std::tuple<ParameterType, std::any>> Parameters;
    //		 Kind	     		   Parameter	its value	
    std::map<std::string, std::map<std::string, std::any>> ConditionKinds;
    // 		Variant				   Parameter	its value
    std::map<std::string, std::map<std::string, std::any>> ConditionVariants;
    std::tuple<std::string, std::string> DefaultAnimation;
    //                            path to texture
    static std::vector<std::pair<std::string, Texture2D>> LoadedTextures;

    AnimationManager() = default;
    //AnimationManager(const AnimationManager& manager);

    void SetParameter(std::string name, std::any value);
    AnimatedSprite* GetSprite();
    bool CompareParameter(std::string name, std::any value);
    static void DeleteAllTextures();    
    void Update(float dt);

    // Animation control.
    /// Select last variant if (variant == "").
    void PlayOnce(std::string kind, std::string variant = "");
    void Play(std::string kind, std::string variant = "");
    void Stop(std::string kind, std::string variant = "");

    void SetLastDifferentAnimation(std::tuple < std::string, std::string> a) { last_animation = a; }

protected:
    bool playing = false;        
    bool playing_loop = false;
    AnimatedSprite* playing_sprite = nullptr;

    std::tuple<std::string, std::string> last_animation;

    void onPlayingAnimationEnd(AnimatedSprite* sprite);
    void resetLastAnimation();
};

void to_json(nlohmann::json& j, const AnimatedSprite& s);
void from_json(const nlohmann::json& j, AnimatedSprite& s);
void from_json(const nlohmann::json& j, AnimationManager& m);
void to_json(nlohmann::json& j, const AnimationManager& m);