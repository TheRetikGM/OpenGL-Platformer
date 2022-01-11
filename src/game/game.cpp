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
#include "TextRenderer.h"
#include "Helper.hpp"
#include "menu.hpp"
#include <GLFW/glfw3.h>
#include <thread>
#define printf_v(name, vec, prec) std::printf(name ": [%" prec "f, %" prec "f]\n", (vec).x, (vec).y)

using namespace std::placeholders;

// Initialize static Game member variables.
glm::vec2 Game::TileSize = glm::vec2(32.0f, 32.0f);
std::vector<ITileSpace*> Game::tileSpaceObjects;

SpriteRenderer*	 renderer = nullptr;
TilemapRenderer* tile_renderer = nullptr;
BasicRenderer*	 basic_renderer = nullptr;
TextRenderer* text_renderer = nullptr;

Player* player = nullptr;

// Render state variables.
bool wireframe_render = false;
bool render_aabb = false;
float fps = 0.0f;
float ref_fps = 0.0f;
float n_fps = 0.0f;
float t_fps = 0.0f;

// Menu variables
using namespace MenuSystem;
MenuObject* menu;
MenuRenderer* menu_renderer;
MenuManager* menu_manager;

// Temp
Helper::Stopwatch w1;
Helper::Stopwatch w2;
Helper::Stopwatch w3;

// Callbacks.
void onLayerDraw(const Tmx::Map *map, const Tmx::Layer *layer, int n_layer);
void onCameraScale(glm::vec2 scale);

Game::Game(unsigned int width, unsigned int height) : State(GameState::active), Width(width), Height(height), Keys(), KeysProcessed(), BackgroundColor(0.0f), CurrentLevel(-1)
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
	if (text_renderer)
		delete text_renderer;
	if (menu)
		delete menu;
	if (menu_renderer)
		delete menu_renderer;
	if (menu_manager)
		delete menu_manager;
	
	ResourceManager::Clear();
	for (auto& level : this->Levels)
		GameLevel::Delete(level);
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
	Game::SetTileSize(glm::vec2(32.0f, 32.0f));
	this->BackgroundColor = glm::vec3(0.4f, 0.5f, 0.4f);

	// Load shaders
	ResourceManager::LoadShader(SHADERS_DIR "SpriteRender.vert", SHADERS_DIR "SpriteRender.frag", nullptr, "sprite");
	ResourceManager::LoadShader(SHADERS_DIR "tile_render.vert", SHADERS_DIR "tile_render.frag", nullptr, "tilemap");
	ResourceManager::LoadShader(SHADERS_DIR "BasicRender.vert", SHADERS_DIR "BasicRender.frag", nullptr, "basic_render");

	// Load textures
	ResourceManager::LoadTexture(ASSETS_DIR "sprites/bg_1.png", true, "background1");
	ResourceManager::LoadTexture(ASSETS_DIR "textures/menu_9patch.png", true, "menu_9patch").SetMagFilter(GL_NEAREST).SetMinFilter(GL_NEAREST).UpdateParameters();

	// Load levels
	this->Levels.push_back(GameLevel::Load(ASSETS_DIR "Levels/level_0.lvl"));
	this->CurrentLevel = 0;

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

	// Load Fonts and Initialize text renderer.
	text_renderer = new TextRenderer(Width, Height);
	text_renderer->Load(ASSETS_DIR "fonts/arial.ttf", 16, GL_NEAREST);

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
	player = new Player(glm::vec2(10.0f, 10.0f), glm::vec2(0.7f, 1.4f), playerAnimations->GetSprite(), glm::vec3(1.0f));
	player->SetRigidBody(Physics2D::RigidBody::CreateRectangleBody(player->Position, { 0.7f, 1.4f }, 5.0f, false, 0.0f));
	player->RBody->IsKinematic = true;
	player->RBody->Name = "player";
	player->RBody->Properties.Restitution = 0.0f;
	player->RBody->GravityScale = 1.0f;
	player->RBody->Properties.FrictionCoeff = 0.9f;
	player->IsJumping = false;
	player->Animator = playerAnimations;

	// Set initial states.	
	TileCamera2D::SetFollow(player);
	player->AddToWorld(Levels[CurrentLevel]->PhysicsWorld);

	/* ====== Initizlie menu objects ======= */
	// Initialize menus.
	menu = new MenuObject();
	menu->SetPatchSize(glm::ivec2(16));
	MenuObject& pauseMenu = menu->at("Pause Menu").SetTable(1, 3);
	pauseMenu["Resume"].SetID(101);
	pauseMenu["Options"].SetTable(2, 2);
	pauseMenu["Options"]["option1"];
	pauseMenu["Options"]["option2"];
	pauseMenu["Options"]["option3"];
	pauseMenu["Options"]["option4"];
	pauseMenu["Options"]["option5"];
	pauseMenu["Options"]["option6"];
	pauseMenu["Options"]["option7"];
	pauseMenu["Exit game"].SetID(103);
	pauseMenu.Build(text_renderer);

	// Initilize menu manager
	menu_manager = new MenuManager();

	// Initialize menu renderer
	menu_renderer = new MenuRenderer(renderer);
}

void Game::ProcessInput(float dt)
{
	if (this->State == GameState::active)
	{
		player->ProcessKeyboard(Keys, KeysProcessed, dt);

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
			player->RBody->LinearVelocity = glm::vec2(0.0f);
		}
		if (Keys[GLFW_KEY_F2] && !KeysProcessed[GLFW_KEY_F2])
		{
			render_aabb = !render_aabb;
			KeysProcessed[GLFW_KEY_F2] = true;
		}


		// ==== TODO: move this into player's code
		auto playerAnim = ResourceManager::GetAnimationManager("PlayerAnimations");
		if (Keys[GLFW_KEY_LEFT_SHIFT] && !KeysProcessed[GLFW_KEY_LEFT_SHIFT])
		{
			playerAnim->PlayOnce("attack");
			KeysProcessed[GLFW_KEY_LEFT_SHIFT] = true;
		}

		if (Keys[GLFW_KEY_ESCAPE] && !KeysProcessed[GLFW_KEY_ESCAPE])
		{
			this->State = GameState::ingame_paused;	
			menu_manager->Open(&menu->at("Pause Menu"));
			KeysProcessed[GLFW_KEY_ESCAPE] = true;
		}
	}
	else if (this->State == GameState::ingame_paused)
	{
		if (Keys[GLFW_KEY_W] && !KeysProcessed[GLFW_KEY_W])
		{
			menu_manager->OnUp();
			KeysProcessed[GLFW_KEY_W] = true;
		}
		if (Keys[GLFW_KEY_S] && !KeysProcessed[GLFW_KEY_S])
		{
			menu_manager->OnDown();
			KeysProcessed[GLFW_KEY_S] = true;
		}
		if (Keys[GLFW_KEY_A] && !KeysProcessed[GLFW_KEY_A])
		{
			menu_manager->OnLeft();
			KeysProcessed[GLFW_KEY_A] = true;
		}
		if (Keys[GLFW_KEY_D] && !KeysProcessed[GLFW_KEY_D])
		{
			menu_manager->OnRight();
			KeysProcessed[GLFW_KEY_D] = true;
		}
		if (Keys[GLFW_KEY_SPACE] && !KeysProcessed[GLFW_KEY_SPACE])
		{
			MenuObject* command = menu_manager->OnConfirm();
			if (command)
			{
				if (command->GetID() == 101)
				{
					menu_manager->Close();
					this->State = GameState::active;
				}
				else if (command->GetID() == 103)
				{
					this->Run = false;
				}
				// menu_manager->Close();
			}

			KeysProcessed[GLFW_KEY_SPACE] = true;
		}
		if (Keys[GLFW_KEY_ESCAPE] && !KeysProcessed[GLFW_KEY_ESCAPE])
		{
			menu_manager->OnBack();
			if (menu_manager->MenuClosed())
				this->State = GameState::active;
			KeysProcessed[GLFW_KEY_ESCAPE] = true;
		}
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

}
void Game::Update(float dt)
{
	if (State == GameState::active)
	{
		TileCamera2D::Update(dt);
		w1.Restart();
		Levels[CurrentLevel]->Update(dt);
		w1.Stop();
		player->Update(dt);
		player->SetSprite(player->Animator->GetSprite());

		// Update Animations
		for (auto& [name, manager] : ResourceManager::AnimationManagers)
			manager->Update(dt);
		// player->PlayerSprite = ResourceManager::GetAnimationManager("PlayerAnimations")->GetSprite();
	}
	
	// Step update for FPS.
	if (ref_fps < 2.0f) {
		ref_fps += dt;
		n_fps += 1.0f;
		t_fps += 1.0f / dt;
	}
	else {
		fps = t_fps / n_fps;
		t_fps = 0.0f;
		ref_fps = 0.0f;
		n_fps = 0.0f;
	}
}
void Game::Render()
{
	w2.Restart();
	if (State == GameState::active || State == GameState::ingame_paused)
	{
		renderer->DrawSprite(ResourceManager::GetTexture("background1"), glm::vec2(0.0f, 0.0f), glm::vec2(Width, Height), 0.0f);
		// Render tilemap.		
		tile_renderer->Draw(Levels[CurrentLevel]->Map, glm::vec2(0.0f, 0.0f));		

		// Render colliders.
		if (render_aabb)
		{
			auto world = Levels[CurrentLevel]->PhysicsWorld;
			for (int i = 0; i < world->BodyCount(); i++)
			{
				auto body = world->GetBody(i);
				auto p = body->GetCollider();
				if (body->GetCollider()->GetType() != Physics2D::ColliderType::capsule)
				{
					basic_renderer->RenderClosedPolygon(
						p->UnitVertices,
						TileCamera2D::GetScreenPosition(p->GetCenter()),
						glm::vec2(world->GetInvMeterUnitRatio()) * Game::TileSize * TileCamera2D::GetScale(),
						glm::vec3(1.0f, 1.0f, 1.0f));
				}
				else {
					auto cap = body->GetCollider()->Get<Physics2D::CapsuleCollider*>();
					basic_renderer->RenderShape(br_Shape::circle_empty,
						TileCamera2D::GetScreenPosition(cap->TopCollider->GetAABB().GetMin(true)),
						cap->TopCollider->GetAABB().GetSize(true) * Game::TileSize * TileCamera2D::GetScale(),
						0.0f, glm::vec3(1.0f));
					basic_renderer->RenderShape(br_Shape::rectangle,
						TileCamera2D::GetScreenPosition(cap->MiddleCollider->GetAABB().GetMin(true)),
						cap->MiddleCollider->GetAABB().GetSize(true) * Game::TileSize * TileCamera2D::GetScale(),
						0.0f, glm::vec3(1.0f));
					basic_renderer->RenderShape(br_Shape::circle_empty,
						TileCamera2D::GetScreenPosition(cap->BottomCollider->GetAABB().GetMin(true)),
						cap->BottomCollider->GetAABB().GetSize(true) * Game::TileSize * TileCamera2D::GetScale(),
						0.0f, glm::vec3(1.0f));
				}
			}
			basic_renderer->RenderShape(br_Shape::rectangle, player->GetSprite()->Position, player->GetSprite()->Size, 0.0f, glm::vec3(1.0f, 1.0f, 0.0f));
		}
	}
	if (State == GameState::ingame_paused)
	{
		glm::vec2 vMenuSize = menu_manager->First().GetTotalSize(2.0f);
		menu_renderer->Draw(*menu_manager, ResourceManager::GetTexture("menu_9patch"), (glm::vec2(Width, Height) - vMenuSize) * 0.5f, 2.0f);
	}
	w2.Stop();

	// Render DEBUG text
	char buf[256];
	sprintf(buf, "FPS: %.f\nUpdate step: %f ms\nRender step: %f ms\nSpriteSize: %s",
		fps, 
		w1.ElapsedMilliseconds(),
		w2.ElapsedMilliseconds(),
		("(" + std::to_string(player->GetSprite()->Size.x) + ", " + std::to_string(player->GetSprite()->Size.y) + ")").c_str()
	);
	text_renderer->RenderText(std::string(buf), 10.0f, 10.0f, 1.0f);
}

// Callbacks
/// Called AFTER the layer is drawn.
void Game::OnLayerRendered(const Tmx::Map *map, const Tmx::Layer *layer, int n_layer)
{
	if (layer->GetName() == "entity")
	{
		// Render player.
		player->Draw(renderer);
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	}
}
void onCameraScale(glm::vec2 scale)
{
	tile_renderer->HandleScale(scale);
}
