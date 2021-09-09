#include <iostream>
#include <string>
#include "game/game.h"
#include "resource_manager.h"
#include "config.h"
#include "sprite_renderer.h"
#include "tilemap_renderer.h"
#include "tilemap.h"
#include "camera/tileCamera2D.h"
#include "basic_renderer.h"
#include "game/Player.h"
#include <GLFW/glfw3.h>
using namespace std::placeholders;

// Initialize static Game member variables.
glm::vec2 Game::TileSize = glm::vec2(32.0f, 32.0f);
std::vector<ITileSpace*> Game::tileSpaceObjects;

SpriteRenderer*	 renderer = nullptr;
TilemapRenderer* tile_renderer = nullptr;
BasicRenderer*	 basic_renderer = nullptr;

Player* player = nullptr;

// Render state variables.
bool wireframe_render = false;

// Callbacks.
void onLayerDraw(const Tmx::Map *map, const Tmx::Layer *layer, int n_layer);
void onCameraScale(glm::vec2 scale);

Game::Game(unsigned int width, unsigned int height) : State(GameState::active), Width(width), Height(height), Keys(), KeysProcessed(), BackgroundColor(0.0f)
{}
Game::~Game()
{	
	if (renderer)
		delete renderer;
	if (tile_renderer)
		delete tile_renderer;
	if (basic_renderer)
		delete basic_renderer;
	if (player)
		delete player;
	ResourceManager::Clear();
}
void Game::SetTileSize(glm::vec2 new_size)
{
	Game::TileSize = new_size;
	std::for_each(tileSpaceObjects.begin(), tileSpaceObjects.end(), [&](ITileSpace* obj) { 
		obj->onTileSizeChanged(new_size); 
	});	
}
void Game::OnResize()
{
	// If resized, update projection matrix to match new width and height of window.
	glm::mat4 projection = glm::ortho(0.0f, (float)this->Width, (float)this->Height, 0.0f, -1.0f, 1.0f);
	ResourceManager::GetShader("sprite").Use().SetMat4("projection", projection);
	
	projection = glm::ortho(0.0f, (float)Width, (float)Height, 0.0f, -1.0f, 1.0f);
	tile_renderer->Projection = projection;

	TileCamera2D::ScreenCoords = glm::vec2((float)this->Width, (float)this->Height);

	ResourceManager::GetShader("basic_render").Use().SetMat4("projection", projection);
}
void Game::ProcessMouse(float xoffset, float yoffset)
{

}
void Game::ProcessScroll(float yoffset)
{
	TileCamera2D::SetScale(TileCamera2D::GetScale() + 0.1f * yoffset);
	Game::SetTileSize(Game::TileSize);
}
void Game::Init()
{
	// Load shaders
	ResourceManager::LoadShader(SHADERS_DIR "SpriteRender.vert", SHADERS_DIR "SpriteRender.frag", nullptr, "sprite");
	ResourceManager::LoadShader(SHADERS_DIR "tile_render.vert", SHADERS_DIR "tile_render.frag", nullptr, "tilemap");
	ResourceManager::LoadShader(SHADERS_DIR "BasicRender.vert", SHADERS_DIR "BasicRender.frag", nullptr, "basic_render");

	// Load textures
	ResourceManager::LoadTexture(ASSETS_DIR "tmw_desert_spacing.png", true, "torches");
	ResourceManager::LoadTexture(ASSETS_DIR "sprites/bg_1.png", true, "background1");

	// Load tilemaps
	ResourceManager::LoadTilemap(ASSETS_DIR "example.tmx", "desert");
	ResourceManager::LoadTilemap(ASSETS_DIR "tilemaps/test/test.tmx", "platformer");

	// Load animations
	AnimationManager* playerAnimations = ResourceManager::LoadAnimationManager(ASSETS_DIR "animations/PlayerAnimations_platformer.json");

	// Projection used for 2D projection.
	glm::mat4 projection = glm::ortho(0.0f, (float)this->Width, (float)this->Height, 0.0f, -1.0f, 1.0f);

	// Initialize sprite renderer.		
	ResourceManager::GetShader("sprite").Use().SetInt("spriteImage", 0);
	ResourceManager::GetShader("sprite").SetMat4("projection", projection);
	renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));

	// Initialize basic renderer.
	ResourceManager::GetShader("basic_render").Use().SetMat4("projection", projection);
	basic_renderer = new BasicRenderer(ResourceManager::GetShader("basic_render"));
	basic_renderer->SetLineWidth(2.0f);

	// Initialize Camera	
	TileCamera2D::SetPosition(glm::vec2(0.0f, 0.0f));
	TileCamera2D::SetRight(glm::vec2(1.0f, 0.0f));
	TileCamera2D::ScreenCoords = glm::vec2((float)this->Width, this->Height);
	TileCamera2D::SetScale(glm::vec2(2.0f));
	TileCamera2D::OnScale = onCameraScale;

	// Initialize tileset renderer
	tile_renderer = new TilemapRenderer(ResourceManager::GetShader("tilemap"));
	tile_renderer->Projection = projection;
	tile_renderer->AfterLayer_callback = std::bind(&Game::OnLayerRendered, this, _1, _2, _3);

	// Initialize player
	player = new Player(glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 2.0f), playerAnimations->GetSprite(), glm::vec3(1.0f));
	player->MovementSpeed = 6.0f;	

	// Set initial states.
	//TileCamera2D::Follow = player;
	TileCamera2D::SetFollow(player);

	Game::SetTileSize(glm::vec2(32.0f, 32.0f));
	this->BackgroundColor = glm::vec3(0.3f);
}

void Game::ProcessInput(float dt)
{	
	int horizontal = 0;
	int vertical = 0;
	if (Keys[GLFW_KEY_W])
	{
		TileCamera2D::ProccessKeyboard(Camera2DMovement::up, dt);
		player->ProcessKeyboard(PlayerMovement::up, dt);
		vertical += 1;
	}
	if (Keys[GLFW_KEY_S])
	{
		TileCamera2D::ProccessKeyboard(Camera2DMovement::down, dt);
		player->ProcessKeyboard(PlayerMovement::down, dt);
		vertical -= 1;
	}
	if (Keys[GLFW_KEY_A])
	{
		TileCamera2D::ProccessKeyboard(Camera2DMovement::left, dt);
		player->ProcessKeyboard(PlayerMovement::left, dt);
		horizontal -= 1;
	}
	if (Keys[GLFW_KEY_D])
	{
		TileCamera2D::ProccessKeyboard(Camera2DMovement::right, dt);
		player->ProcessKeyboard(PlayerMovement::right, dt);
		horizontal += 1;
	}
	if (Keys[GLFW_KEY_Q])
	{
		TileCamera2D::Rotate(glm::radians(45.0f) * dt);
	}
	if (Keys[GLFW_KEY_E])
	{
		TileCamera2D::Rotate(glm::radians(-45.0f) * dt);
	}
	if (Keys[GLFW_KEY_SPACE])
	{
		TileCamera2D::SetRight(glm::vec2(1.0f, 0.0f));	
		TileCamera2D::SetScale(glm::vec2(2.0f));	
		Game::SetTileSize(Game::TileSize);	
	}
	if (Keys[GLFW_KEY_F1] && !KeysProcessed[GLFW_KEY_F1])
	{
		wireframe_render = !wireframe_render;
		if (wireframe_render)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		KeysProcessed[GLFW_KEY_F1] = true;		
	}

	auto playerAnim = ResourceManager::GetAnimationManager("PlayerAnimations");
	if (Keys[GLFW_KEY_LEFT_SHIFT] && !KeysProcessed[GLFW_KEY_LEFT_SHIFT])
	{
		playerAnim->PlayOnce("attack");
		KeysProcessed[GLFW_KEY_LEFT_SHIFT] = true;
	}

	if (horizontal == 0 && vertical == 0)
		playerAnim->SetParameter("state", "idle");
	else
	{		
		playerAnim->SetParameter("state", "run");
	}
	/// Behaivor for horizontal == 0 is not defined if not defined! Character must be facing in some direction.
	if (horizontal != 0)
		playerAnim->SetParameter("horizontal", horizontal);
	playerAnim->SetParameter("vertical", vertical);
}
void Game::Update(float dt)
{
	ResourceManager::GetTilemap("platformer")->Update(dt);	
	
	// Does nothing for now.
	/// TileCamera2D::Update(dt);

	for (auto& [name, manager] : ResourceManager::AnimationManagers)
		manager->Update(dt);
	player->PlayerSprite = ResourceManager::GetAnimationManager("PlayerAnimations")->GetSprite();	
}
void Game::Render()
{
	if (State == GameState::active)
	{
		renderer->DrawSprite(ResourceManager::GetTexture("background1"), glm::vec2(0.0f, 0.0f), glm::vec2(Width, Height), 0.0f);
		// Render tilemap.		
		tile_renderer->Draw(ResourceManager::GetTilemap("platformer"), glm::vec2(0.0f, 0.0f));		

		// printf("Pos: [%3.7f, %3.7f]; ScreenPos: [%3.7f, %3.7f]\n", player->Position.x, player->Position.y, player->ScreenPosition.x, player->ScreenPosition.y);
	}
}

// Callbacks
/// Called AFTER the layer is drawn.
void Game::OnLayerRendered(const Tmx::Map *map, const Tmx::Layer *layer, int n_layer)
{
	if (layer->GetName() == "entity")
	{
		// Render player.
		player->Draw(renderer);
		/// Render bounding box around player sprite.
		basic_renderer->RenderShape(br_Shape::rectangle, player->PlayerSprite->Position, player->PlayerSprite->Size, 0.0f, glm::vec3(1.0f, 1.0f, 0.0f));		
	}
}
void onCameraScale(glm::vec2 scale)
{
	tile_renderer->HandleScale(scale);
}