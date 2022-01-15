#include "game/GameStates.h"
#include "config.h"
#include "resource_manager.h"
#include "shader.h"
#include "game/game.h"
#include "camera/tileCamera2D.h"
#include <functional>

using namespace std::placeholders;

void InLevelState::Init()
{
    // Load shaders.
	Shader tilemapShader = ResourceManager::LoadShader(SHADERS_DIR "tile_render.vert", SHADERS_DIR "tile_render.frag", nullptr, "drawlevel_tilemap");
    Shader spriteShader = ResourceManager::LoadShader(SHADERS_DIR "SpriteRender.vert", SHADERS_DIR "SpriteRender.frag", nullptr, "drawlevel_sprite");

    // Shader initial states.
    spriteShader.Use().SetInt("spriteImage", 0);
    spriteShader.SetMat4("projection", Game::ProjectionMatrix);

    // Load textures.
    ResourceManager::LoadTexture(ASSETS_DIR "sprites/bg_1.png", true, "bg1");
    // Load player animations.
    auto playerAnimations = ResourceManager::LoadAnimationManager(ASSETS_DIR "animations/PlayerAnimations_platformer.json");

    // Create sprite and tilemap renderer.
    pTilemapRenderer = new TilemapRenderer(tilemapShader);
    pTilemapRenderer->Projection = Game::ProjectionMatrix;
    pTilemapRenderer->AfterLayer_callback = std::bind(InLevelState::OnLayerRendered, this, _1, _2, _3);

    // Create player.
    pPlayer = new Player(
        glm::vec2(10.0f, 10.0f), 
        glm::vec2(0.7f, 1.4f), 
        playerAnimations->GetSprite(), 
        glm::vec3(1.0f)
    );
    pPlayer->SetRigidBody(Physics2D::RigidBody::CreateRectangleBody(pPlayer->Position, { 0.7f, 1.4f }, 5.0f, false, 0.0f));
	pPlayer->RBody->IsKinematic = true;
	pPlayer->RBody->Name = "player";
	pPlayer->IsJumping = false;
	pPlayer->Animator = playerAnimations;

    TileCamera2D::SetFollow(pPlayer);
    TileCamera2D::OnScale = [&](glm::vec2 vScale) { this->pTilemapRenderer->HandleScale(vScale); };
    pPlayer->AddToWorld(pLevel->PhysicsWorld);
}
void InLevelState::OnExit()
{
    if (pTilemapRenderer)
        delete pTilemapRenderer;
    if (pSpriteRenderer)
        delete pSpriteRenderer;
    ResourceManager::DeleteShader("drawlevel_tilemap");
    ResourceManager::DeleteShader("drawlevel_sprite");
    ResourceManager::DeleteTexture("bg1");
    ResourceManager::DeleteAnimationManager(pPlayer->Animator->Name);
    delete pPlayer;
}

void InLevelState::Render()
{
    pSpriteRenderer->DrawSprite(ResourceManager::GetTexture("bg1"), {0.0f, 0.0f}, Game::ScreenSize, 0.0f);
    pTilemapRenderer->Draw(pLevel->Map, {0.0f, 0.0f});
}

void InLevelState::OnLayerRendered(const Tmx::Map *map, const Tmx::Layer *layer, int n_layer)
{
    if (layer->GetName() == "entity")
    {
        // Render player.
        pPlayer->Draw(pSpriteRenderer);
    }
}