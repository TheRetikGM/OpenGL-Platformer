#pragma once
#include "texture.h"
#include <glm/glm.hpp>
#include "shader.h"
#include "sprite_renderer.h"
#include "Interfaces/ITileSpace.h"

class Sprite : public ITileSpace
{
public:
    glm::vec2   Position;
    glm::vec2   Size;    
    Texture2D   Texture;
    glm::vec3   Color;
    float       Rotation;
    bool        FlipTex_x;
    bool        FlipTex_y;

    Sprite (glm::vec2 position, glm::vec2 size, Texture2D texture);
    Sprite();
    ~Sprite();

    virtual void Draw(SpriteRenderer* renderer);
    void onTileSizeChanged(glm::vec2 newTileSize) override;
};