#include "camera/tileCamera2D.h"
#include <glm/gtc/type_ptr.hpp>
#include <exception>
#include <stdexcept>
#include <game/game.h>

/* Initialize static member variables */
glm::vec2 TileCamera2D::position = glm::vec2(0.0f);
glm::vec2* TileCamera2D::Position = &TileCamera2D::position;
float TileCamera2D::MoveSpeed = 0.0f;
std::function<void (glm::vec2)> TileCamera2D::OnScale = [](glm::vec2 scale){};
glm::vec2 TileCamera2D::ScreenCoords = glm::vec2(0.0f);
GameObject* TileCamera2D::Follow = nullptr;
glm::vec2 TileCamera2D::right = glm::vec2(1.0f, 0.0f);
glm::vec2 TileCamera2D::scale = glm::vec2(1.0f);
glm::vec2 TileCamera2D::CurrentMapSize = TileCamera2D::ScreenCoords;

glm::mat4 TileCamera2D::GetViewMatrix()
{
    glm::vec2 CameraCenter = ScreenCoords / 2.0f;
    float r_mat[] = {
        right.x, -right.y, 0.0f, 0.0f,
        right.y, right.x, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    glm::mat4 view(1.0f);
    view = glm::translate(view, glm::vec3(CameraCenter, 0.0f));
    view = view * glm::make_mat4(r_mat);
    view = glm::translate(view, glm::vec3(-CameraCenter, 0.0f));

    return view;
}
void TileCamera2D::ProccessKeyboard(Camera2DMovement direction, float delta)
{
    if (Follow)
        return;
    glm::vec2 down(-right.y, right.x);
    switch (direction)
    {
    case Camera2DMovement::up:
        *Position -= down * MoveSpeed * delta;
        break;
    case Camera2DMovement::down:
        *Position += down * MoveSpeed * delta;
        break;
    case Camera2DMovement::left:
        *Position -= right * MoveSpeed * delta;
        break;
    case Camera2DMovement::right:
        *Position += right * MoveSpeed * delta;
        break;
    }
}
void TileCamera2D::Rotate(float angle)
{
    angle *= -1.0f;
    glm::mat2 rot = {
        glm::vec2(std::cos(angle), std::sin(angle)),
        glm::vec2(-std::sin(angle), std::cos(angle))
    };
    right = rot * right;
}
void TileCamera2D::SetRight(glm::vec2 right)
{
    TileCamera2D::right = right;    
}
glm::vec2 TileCamera2D::GetFirstVisibleTile()
{
    glm::ivec2 nVisibleTiles = ScreenCoords / (Game::TileSize * scale);
    glm::vec2 offset;    

    // Calculate Top-Leftmost tile.
    offset = *Position - ScreenCoords / (Game::TileSize * scale * 2.0f);    
    
    glm::vec2 tile_overflow = CurrentMapSize / Game::TileSize - glm::vec2(glm::ivec2(CurrentMapSize / Game::TileSize));

    // Clamp camera to map boundaries;    
    if (offset.x < 0) offset.x = 0;
    if (offset.y < 0) offset.y = 0;
    if (offset.x > CurrentMapSize.x - nVisibleTiles.x - tile_overflow.x) 
        offset.x = (float)(CurrentMapSize.x - nVisibleTiles.x) - tile_overflow.x;
    if (offset.y > CurrentMapSize.y - nVisibleTiles.y - tile_overflow.y) 
        offset.y = (float)(CurrentMapSize.y - nVisibleTiles.y) - tile_overflow.y;

    if (CurrentMapSize.x < nVisibleTiles.x)
        offset.x = 0;
    if (CurrentMapSize.y < nVisibleTiles.y)
        offset.y = 0;
    
    return offset;
}
glm::ivec2 TileCamera2D::GetNVisibleTiles()
{
    return ScreenCoords / (Game::TileSize * scale);
}
void TileCamera2D::SetScale(glm::vec2 scale)
{
    if (scale.x > 0.0f && scale.y > 0.0f)
        TileCamera2D::scale = scale;
    else
        TileCamera2D::scale = glm::vec2(0.0f);
    OnScale(TileCamera2D::scale);
}
glm::vec2 TileCamera2D::GetScale()
{
    return TileCamera2D::scale;
}
void TileCamera2D::RenderAtPosition(BasicRenderer* renderer, br_Shape shape, glm::vec3 color)
{
    glm::vec2 size = scale * 10.0f;
    glm::vec2 pos = *Position * Game::TileSize * scale - size / 2.0f;
    renderer->RenderShape(shape, pos, size, 0.0f, color);
}


void TileCamera2D::Update(float dt)
{
    //glm::vec2 old_to_new = Follow->Position - position;
    //glm::vec2 middle_point = GetTileSpacePosition(ScreenCoords / 2.0f);
    //glm::vec2 move_field_size = (ScreenCoords / 4.0f) / (Game::TileSize * TileCamera2D::scale);
    //glm::vec2 move_field_o = middle_point - move_field_size / 2.0f;

    //glm::vec2 t_near = (move_field_o - position) / old_to_new;
    //glm::vec2 t_far = (move_field_o + move_field_size - position) / old_to_new;

    //if (t_near.x > t_far.x) std::swap(t_near.x, t_far.x);
    //if (t_near.y > t_far.y) std::swap(t_near.y, t_far.y);

    //if (t_near.x > t_far.x || t_near.y > t_far.y)
    //    return;

    //float t_hit_near = std::max(t_near.x, t_near.y);
    //float t_hit_far = std::min(t_far.x, t_far.y);

    //if (t_hit_far < 0.0f || t_hit_far > 1.0f)
    //    return;

    //position = position + (1.0f - t_hit_far) * old_to_new;

    // Move camera quadratically to player position.
    glm::vec2 relative_position = Follow->Position - position;
    glm::vec2 sign = glm::vec2(relative_position.x < 0.0f ? -1.0f : 1.0f, relative_position.y < 0.0f ? -1.0f : 1.0f);
    glm::vec2 move_step = sign * relative_position * relative_position * dt * 2.0f;
    
    // If the distance to player is small move to the player position in 1 / 3 of a second.
    auto in_range = [](const float& val, float a, float b) { return val >= a && val <= b; };
    if (in_range(move_step.x, -0.001f, 0.001f) || in_range(move_step.y, -0.001f, 0.001f))
        move_step = relative_position * dt * 3.0f;

    position += relative_position;
}


glm::vec2 TileCamera2D::GetScreenPosition(glm::vec2 tileSpacePosition)
{    
    return (tileSpacePosition - GetFirstVisibleTile()) * Game::TileSize * TileCamera2D::scale;
}
glm::vec2 TileCamera2D::GetTileSpacePosition(glm::vec2 screenSpacePosition)
{
    return (screenSpacePosition / (Game::TileSize * TileCamera2D::scale)) + GetFirstVisibleTile();
}

void TileCamera2D::SetFollow(GameObject* follow_obj)
{
    // Position = &follow_obj->Position;
    Follow = follow_obj;
}
void TileCamera2D::UnsetFollow(bool keep_position)
{
    if (keep_position)
        position = *Position;
    Position = &position;
}
void TileCamera2D::SetPosition(glm::vec2 pos)
{
    position = pos;
}