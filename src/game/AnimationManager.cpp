#include "game/AnimationManager.h"
#include <fstream>
#include <sstream>
#include <stb_image.h>
#include <glad/glad.h>
#include <iostream>
#include "config.h"

using json = nlohmann::json;

std::vector<std::pair<std::string, Texture2D>> AnimationManager::LoadedTextures;

void AnimationManager::SetParameter(std::string name, std::any value)
{
    if (Parameters.find(name) == Parameters.end())
        throw std::runtime_error(("Parameter '" + name + "' not found.").c_str());
    ParameterType type = std::get<0>(Parameters[name]);
    if (type == ParameterType::p_string)
    {
        if (std::string(value.type().name()).find("char const") != std::string::npos
            || std::string(value.type().name()).find("PKc") != std::string::npos)
        {
            std::get<1>(Parameters[name]) = std::string(std::any_cast<const char*>(value));
        }
        else
            std::get<1>(Parameters[name]) = value;
    }
    else 
        std::get<1>(Parameters[name]) = value;
}
AnimatedSprite* AnimationManager::GetSprite()
{
    if (playing)
        return playing_sprite;

    std::string animKind = "", animVariant = "";
    for (auto& [kind, parameters] : ConditionKinds)
    {
        bool selectThis = true;
        for (auto& [paramName, paramValue] : parameters)
        {
            if (!CompareParameter(paramName, paramValue)) {						
                selectThis = false;
                break;
            }
        }
        if (selectThis) {
            animKind = kind;
            break;
        }
    }
    for (auto& [variant, parameters] : ConditionVariants)
    {
        bool selectThis = true;
        for (auto& [paramName, paramValue] : parameters)
        {
            if (!CompareParameter(paramName, paramValue)) {
                selectThis = false;
                break;
            }
        }
        if (selectThis) {
            animVariant = variant;
        }
    }
    // If animation changed
    if (animKind != std::get<0>(last_animation) || animVariant != std::get<1>(last_animation))
    {   
        // Reset frames of the last animation.
        if (std::get<0>(last_animation) != "" && std::get<1>(last_animation) != "")
            Animations[std::get<0>(last_animation)][std::get<1>(last_animation)].ResetAnimation();        
    }    
    if (animKind == "" || animVariant == "")
    {
        last_animation = DefaultAnimation;
        return &Animations[std::get<0>(DefaultAnimation)][std::get<1>(DefaultAnimation)];
    }
    
    last_animation = std::make_tuple(animKind, animVariant);
    return &Animations[animKind][animVariant];
}
bool AnimationManager::CompareParameter(std::string name, std::any value)
{
    if (std::get<0>(Parameters[name]) == ParameterType::p_int)
        return std::any_cast<int>(std::get<1>(Parameters[name])) == std::any_cast<int>(value);
    else if (std::get<0>(Parameters[name]) == ParameterType::p_float)
        return std::any_cast<float>(std::get<1>(Parameters[name])) == std::any_cast<float>(value);
    else if (std::get<0>(Parameters[name]) == ParameterType::p_bool)
        return std::any_cast<bool>(std::get<1>(Parameters[name])) == std::any_cast<bool>(value);
    else if (std::get<0>(Parameters[name]) == ParameterType::p_string)
    {
        std::string s1 = std::any_cast<std::string>(std::get<1>(Parameters[name]));
        std::string s2 = std::any_cast<std::string>(value);
        return  s1 == s2;
    }
    return false;
}
void AnimationManager::DeleteAllTextures()
{
    //for (auto& [kind, variants] : Animations)
    //    for (auto& [variant, sprite] : variants)
    //        for (auto& frame : sprite.Frames)
    //            glDeleteTextures(1, &frame.texture.ID);
    for (auto& [path, texture] : AnimationManager::LoadedTextures)
        glDeleteTextures(1, &texture.ID);
}
void AnimationManager::resetLastAnimation()
{
    if (std::get<0>(last_animation) != "" && std::get<1>(last_animation) != "")
        Animations.at(std::get<0>(last_animation)).at(std::get<1>(last_animation)).ResetAnimation();
}
void AnimationManager::PlayOnce(std::string kind, std::string variant)
{
    resetLastAnimation();
    if (variant == "")
    {
        if (std::get<1>(this->last_animation) == "")
            throw std::runtime_error("AnimationManager::PlayOnce(): Last animation variant wanted, but no animation has been played yet.");
        variant = std::get<1>(this->last_animation);        
    }    
    this->playing_sprite = &Animations.at(kind).at(variant);
    this->playing = true;
    this->playing_loop = false;
    this->playing_sprite->onAnimationEnd = std::bind(&AnimationManager::onPlayingAnimationEnd, this, std::placeholders::_1);
    this->playing_sprite->ResetAnimation();
}
void Play(std::string kind, std::string variant);
void Stop(std::string kind, std::string variant);
void AnimationManager::onPlayingAnimationEnd(AnimatedSprite* sprite)
{
    this->playing = false;
    sprite->onAnimationEnd = [&](AnimatedSprite* sprite){};    
    this->playing_sprite = nullptr;
}
void AnimationManager::Update(float dt)
{
    if (!playing)
    {
        auto spr = GetSprite();
        spr->Update(dt);
        //GetSprite()->Update(dt);
    }
    else
    {
        playing_sprite->Update(dt);
    }
}

void to_json(json& j, const AnimatedSprite& s)
{
    j["duration"] = s.Duration;
    j["mirrored"] = s.FlipTex_x;
    if (s.Spritesheet)
    {
        j["spritesheet"]["sprite_width"] = s.Sheet_spriteWidth;
        j["spritesheet"]["sprite_height"] = s.Sheet_spriteHeight;
        j["spritesheet"]["offset"] = s.Sheet_offset;
        j["spritesheet"]["margin"] = s.Sheet_margin;
        j["spritesheet"]["source"] = s.Sheet_source;

    }
    else
    {
        for (int i = 0; i < s.GetFrames().size(); i++)
            j["frames"][i] = s.GetFrames()[i].path;
    }
}

void from_json(const json& j, AnimatedSprite& s)
{
    std::vector<Texture2D> textures;
    std::vector<std::string> paths;
    int width, height, nrChannels;
    if (j.contains("spritesheet"))
    {
        s.Spritesheet = true;
        auto& spritesheet = j.at("spritesheet");        
        spritesheet.at("sprite_width").get_to(s.Sheet_spriteWidth);
        spritesheet.at("sprite_height").get_to(s.Sheet_spriteHeight);
        spritesheet.at("offset").get_to(s.Sheet_offset);
        spritesheet.at("margin").get_to(s.Sheet_margin);
        spritesheet.at("source").get_to(s.Sheet_source);


        std::string directory = ASSETS_DIR "animations/";
        std::string s_file = directory + s.Sheet_source;
        const char* file = s_file.c_str();
        unsigned char* sheet_data = stbi_load(file, &width, &height, &nrChannels, 0);
        if (!sheet_data)
            throw std::runtime_error(("Could not load spritesheet '" + s_file + "'.").c_str());

        auto GetTexel = [&](int x, int y) { return &sheet_data[(y * width + x) * nrChannels]; };

        int columns = (width + s.Sheet_margin - 2 * s.Sheet_offset) / (s.Sheet_spriteWidth + s.Sheet_margin);
        int rows = (height + s.Sheet_margin - 2 * s.Sheet_offset) / (s.Sheet_spriteHeight + s.Sheet_margin);

        unsigned char* new_sprite_data = new unsigned char[s.Sheet_spriteWidth * s.Sheet_spriteHeight * nrChannels];
        for (int x = 0; x < columns; x++)
        {
            for (int y = 0; y < rows; y++)
            {
                std::string localPath = s.Sheet_source + "_" + std::to_string(x) + "_" + std::to_string(y);
                // Check if texture is already loaded.
                auto loadedTexture = std::find_if(AnimationManager::LoadedTextures.begin(), AnimationManager::LoadedTextures.end(), [&](std::pair<std::string, Texture2D>& t) {
                    return t.first == localPath;
                });
                if (loadedTexture != AnimationManager::LoadedTextures.end()) {
                    textures.push_back(loadedTexture->second);
                    paths.push_back(localPath);
                    continue;
                }              

                auto CopySprite = [&](int x, int y) {
                    for (int r = 0; r < s.Sheet_spriteHeight; r++)
                        memcpy(new_sprite_data + r * s.Sheet_spriteWidth * nrChannels, GetTexel(x * s.Sheet_spriteWidth, y + r), nrChannels * s.Sheet_spriteWidth * sizeof(unsigned char));
                };

                CopySprite(x, y);

                Texture2D texture;
                if (nrChannels == 4)
                    texture.Internal_format = texture.Image_format = GL_RGBA;
                texture.Filter_mag = texture.Filter_min = GL_NEAREST;
                texture.Generate(s.Sheet_spriteWidth, s.Sheet_spriteHeight, new_sprite_data);

                textures.push_back(texture);
                paths.push_back(file);
                AnimationManager::LoadedTextures.push_back(std::make_pair(localPath, texture));
            }
        }
        delete[] new_sprite_data;
        stbi_image_free(sheet_data);
    }
    else
    {
        s.Spritesheet = false;
        for (auto& frame : j.at("frames"))
        {
            std::string directory = ASSETS_DIR "animations/";
            std::string test = directory + frame.get<std::string>();
            const char* file = test.c_str();

            auto loadedTexture = std::find_if(AnimationManager::LoadedTextures.begin(), AnimationManager::LoadedTextures.end(), [&](std::pair<std::string, Texture2D>& t) {
                return t.first == frame.get<std::string>();
            });
            if (loadedTexture != AnimationManager::LoadedTextures.end()) {
                textures.push_back(loadedTexture->second);
                paths.push_back(file);
                continue;
            }

            //stbi_set_flip_vertically_on_load(1);
            unsigned char* data = stbi_load(file, &width, &height, &nrChannels, 0);

            Texture2D texture;
            if (nrChannels == 4)
            {
                texture.Internal_format = GL_RGBA;
                texture.Image_format = GL_RGBA;
            }
            texture.Filter_mag = texture.Filter_min = GL_NEAREST;

            if (!data)
                throw std::runtime_error(("Could not load animation frame '" + std::string(file) + "'.").c_str());
            texture.Generate(width, height, data);
            stbi_image_free(data);
            textures.push_back(texture);
            paths.push_back(frame.get<std::string>());
            AnimationManager::LoadedTextures.push_back(std::make_pair(frame.get<std::string>(), texture));
        }
    }
    
    j.at("mirrored").get_to(s.FlipTex_x);
    int duration = j.at("duration").get<int>();
    AnimatedSprite spr(glm::vec2(0.0f), glm::vec2(1.0f), textures, duration);
    for (size_t i = 0; i < spr.Frames.size(); i++)    
        spr.Frames[i].path = paths[i];

    spr.Spritesheet = s.Spritesheet;
    spr.Sheet_spriteWidth = s.Sheet_spriteWidth;
    spr.Sheet_spriteHeight = s.Sheet_spriteHeight;
    spr.Sheet_margin = s.Sheet_margin;
    spr.Sheet_offset = s.Sheet_offset;
    spr.Sheet_source = s.Sheet_source;
    spr.FlipTex_x = s.FlipTex_x;
    spr.FlipTex_y = s.FlipTex_y;
    s = spr;
}
void from_json(const json& j, AnimationManager& m)
{
    m.Name = j.at("name").get<std::string>();

    for (auto& [kind, kindTypes] : j.at("animations").items())		
        for (auto& [variant, animSpriteJson] : kindTypes.items())			
            m.Animations[kind][variant] = animSpriteJson.get<AnimatedSprite>();			

    for (auto& [name, type] : j.at("parameters").items())
    {
        if (type == "string") m.Parameters[name] = std::make_tuple(ParameterType::p_string, std::string(""));
        else if (type == "float") m.Parameters[name] = std::make_tuple(ParameterType::p_float, 0.0f);
        else if (type == "bool") m.Parameters[name] = std::make_tuple(ParameterType::p_bool, false);
        else m.Parameters[name] = std::make_tuple(ParameterType::p_int, 0);
    }
    for (auto& [kind, parameters] : j.at("conditions").at("kind").items())
    {
        for (auto& [paramName, value] : parameters.items())
        {
            if (std::get<0>(m.Parameters[paramName]) == ParameterType::p_int) 		 m.ConditionKinds[kind][paramName] = value.get<int>();
            else if (std::get<0>(m.Parameters[paramName]) == ParameterType::p_float)  m.ConditionKinds[kind][paramName] = value.get<float>();
            else if (std::get<0>(m.Parameters[paramName]) == ParameterType::p_string) m.ConditionKinds[kind][paramName] = value.get<std::string>();
            else if (std::get<0>(m.Parameters[paramName]) == ParameterType::p_bool) 	 m.ConditionKinds[kind][paramName] = value.get<bool>();
        }
    }
    for (auto& [variant, parameters] : j.at("conditions").at("variant").items())
    {
        for (auto& [paramName, value] : parameters.items())
        {
            if (std::get<0>(m.Parameters[paramName]) == ParameterType::p_int) 		 m.ConditionVariants[variant][paramName] = value.get<int>();
            else if (std::get<0>(m.Parameters[paramName]) == ParameterType::p_float)  m.ConditionVariants[variant][paramName] = value.get<float>();
            else if (std::get<0>(m.Parameters[paramName]) == ParameterType::p_string) m.ConditionVariants[variant][paramName] = value.get<std::string>();
            else if (std::get<0>(m.Parameters[paramName]) == ParameterType::p_bool)   m.ConditionVariants[variant][paramName] = value.get<bool>();
        }
    }
    m.DefaultAnimation = std::make_tuple(j.at("defaultAnimation").at("kind").get<std::string>(), j.at("defaultAnimation").at("variant").get<std::string>());
    m.SetLastDifferentAnimation(m.DefaultAnimation);
    int a = 0;
}
void to_json(json& j, const AnimationManager& m)
{
    j["name"] = m.Name;
    for (auto& [kind, kindType] : m.Animations)
    {
        for (auto& [variant, sprite] : kindType)
        {
            j["animations"][kind][variant] = json(sprite);
        }
    }
    for (auto& [parameter, type] : m.Parameters)
    {
        if (std::get<0>(type) == ParameterType::p_int) j["parameters"][parameter] = "int";
        else if (std::get<0>(type) == ParameterType::p_float) j["parameters"][parameter] = "float";
        else if (std::get<0>(type) == ParameterType::p_bool) j["parameters"][parameter] = "bool";
        else if (std::get<0>(type) == ParameterType::p_string) j["parameters"][parameter] = "string";
    }		
    for (auto& [kind, parameters] : m.ConditionKinds)
    {			
        for (auto& [paramName, value] : parameters)
        {	
            if (std::get<0>(m.Parameters.at(paramName)) == ParameterType::p_int) j["conditions"]["kind"][kind][paramName] = std::any_cast<int>(value);
            else if (std::get<0>(m.Parameters.at(paramName)) == ParameterType::p_float) j["conditions"]["kind"][kind][paramName] = std::any_cast<float>(value);
            else if (std::get<0>(m.Parameters.at(paramName)) == ParameterType::p_bool) j["conditions"]["kind"][kind][paramName] = std::any_cast<bool>(value);
            else if (std::get<0>(m.Parameters.at(paramName)) == ParameterType::p_string) j["conditions"]["kind"][kind][paramName] = std::any_cast<std::string>(value);
        }
    }
    for (auto& [variant, parameters] : m.ConditionVariants)
    {
        for (auto& [paramName, value] : parameters)
        {	
            if (std::get<0>(m.Parameters.at(paramName)) == ParameterType::p_int) j["conditions"]["variant"][variant][paramName] = std::any_cast<int>(value);
            else if (std::get<0>(m.Parameters.at(paramName)) == ParameterType::p_float) j["conditions"]["variant"][variant][paramName] = std::any_cast<float>(value);
            else if (std::get<0>(m.Parameters.at(paramName)) == ParameterType::p_bool) j["conditions"]["variant"][variant][paramName] = std::any_cast<bool>(value);
            else if (std::get<0>(m.Parameters.at(paramName)) == ParameterType::p_string) j["conditions"]["variant"][variant][paramName] = std::any_cast<std::string>(value);
        }
    }
    j["defaultAnimation"]["kind"] = std::get<0>(m.DefaultAnimation);
    j["defaultAnimation"]["variant"] = std::get<1>(m.DefaultAnimation);
}