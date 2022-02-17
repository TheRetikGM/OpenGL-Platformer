#pragma once
#include "sprite_renderer.h"
#include <string>
#include <unordered_map>
#include <cctype>

class AtlasTextRenderer
{
public:
    SpriteRenderer* pSpriteRenderer;
    std::unordered_map<char, glm::vec4> Characters;
    glm::vec2 vCharSize = glm::vec2(1.0f, 1.0f);
    Texture2D Atlas;
    int Spacing = 1;

    AtlasTextRenderer() {}

    void Load(Texture2D atlas, glm::vec2 vCharSize)
    {
        Characters.clear();
        this->vCharSize = vCharSize;
        this->Atlas = atlas;
        // Size in characters.
        int atlas_width = atlas.Width / (unsigned int)vCharSize.x;

        int offset = 0;
        const auto GetOffset = [&](int i) {
            glm::ivec2 o((offset + i) % atlas_width, (offset + i) / atlas_width);
            return glm::vec2(o.x * vCharSize.x, o.y * vCharSize.y);
        };

        // Load numbers.
        for (int i = 0; i < 10; i++)
            Characters['0' + i] = glm::vec4(GetOffset(i), vCharSize);
        offset = 10;

        // Load characters.
        for (int i = 0; (i + 'a') <= 'z'; i++)
            Characters['a' + i] = glm::vec4(GetOffset(i), vCharSize);
        offset += 'z' - 'a' + 1;

        // Load other characters.
        char other[4] = {'!', '?', '/', ' '};
        for (int i = 0; i < 4; i++)
            Characters[other[i]] = glm::vec4(GetOffset(i), vCharSize);
    }
    void RenderText(SpriteRenderer* pSpriteRenderer, std::string sText, glm::vec2 vPosition, glm::vec2 vScale = glm::vec2(1.0f), glm::vec3 vColor = glm::vec3(1.0f))
    {
        for (size_t i = 0; i < sText.length(); i++)
        {
            char c = std::tolower(sText[i]);
            glm::vec4 vPartInfo = Characters[c];
            glm::vec2 part_offset(vPartInfo.x, vPartInfo.y);
            glm::vec2 part_size(vPartInfo.z, vPartInfo.w);

            glm::vec2 size = vCharSize;
            pSpriteRenderer->DrawPartialSprite(Atlas, part_offset, part_size, vPosition, size * vScale, 0.0f, vColor);
            vPosition.x += (size.x + Spacing) *  vScale.x;
        }
    }
    inline glm::vec2 GetStringSize(std::string sText, glm::vec2 vScale = glm::vec2(1.0f))
    {
        glm::vec2 vTotalSize(0.0f);
        vTotalSize.x = float(sText.length()) * (vCharSize.x + Spacing) * vScale.x;
        vTotalSize.y = vCharSize.y * vScale.y;

        return vTotalSize;
    }
};