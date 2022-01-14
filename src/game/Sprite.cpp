#include "game/Sprite.h"
#include <glad/glad.h>
#include "game/game.h"

Sprite::Sprite(glm::vec2 position, glm::vec2 size, Texture2D texture)
    : Position(position)
    , Size(size)
    , Texture(texture)
    , Color(1.0f, 1.0f, 1.0f)
    , Rotation(0.0f)
    , FlipTex_x(false)
    , FlipTex_y(false)
{
}
Sprite::Sprite()
{
}
Sprite::~Sprite()
{
}

void Sprite::Draw(SpriteRenderer* renderer)
{
    renderer->GetShader().Use().SetInt("inverse_tex_x", (int)FlipTex_x); 
    renderer->GetShader().SetInt("inverse_tex_y", (int)FlipTex_y);
    renderer->DrawSprite(Texture, Position, Size, Rotation, Color);
    renderer->GetShader().SetInt("inverse_tex_x", 0);
    renderer->GetShader().SetInt("inverse_tex_y", 0);
}
void Sprite::onTileSizeChanged(glm::vec2 newTileSize)
{
    // Set size in tiles.
    Size.x = Texture.Width / newTileSize.x;
    Size.y = Texture.Height / newTileSize.y;
}