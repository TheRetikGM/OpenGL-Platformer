#include <iostream>
#include <string>
#include "game/game.h"
#include "resource_manager.h"
#include "config.h"
#include "sprite_renderer.h"
#include "tilemap_renderer.h"
#include "tilemap.h"
#include "tileCamera2D.h"
#include "basic_renderer.h"
#include "game/Player.h"
#include "TextRenderer.h"
#include "Helper.hpp"
#include "MenuSystem.hpp"
#include "game/GameLevelsManager.h"
#include <GLFW/glfw3.h>
#include <thread>
#include "MenuInputHandler.hpp"
#include "CommandIDs.h"
#include "game/Forms.hpp"
#include "Filesystem.h"
#include "PostProcessor.h"

using namespace std::placeholders;

const glm::vec3 MENU_COMPLETED_LEVEL_COLOR = Helper::HexToRGB(0x2b9f28);

// Initialize static Game member variables.
glm::vec2 Game::TileSize = glm::vec2(32.0f, 32.0f);
glm::mat4 Game::ProjectionMatrix = glm::mat4(1.0f);
glm::vec2 Game::ScreenSize = glm::vec2(500.0f, 800.0f);

SpriteRenderer*	 renderer = nullptr;
TilemapRenderer* tile_renderer = nullptr;
BasicRenderer*	 basic_renderer = nullptr;
TextRenderer* text_renderer = nullptr;
AtlasTextRenderer* atlas_text_renderer = nullptr;

GameLevelsManager* levels_manager = nullptr;

// Render state variables.
bool wireframe_render = false;
bool render_aabb = false;
bool bRenderDebugInfo = false;
float fps = 0.0f;
float ref_fps = 0.0f;
float n_fps = 0.0f;
float t_fps = 0.0f;

// Menu variables
using namespace MenuSystem;
MenuObject* menu;
MenuRenderer* menu_renderer;
MenuManager* menu_manager;
MenuInputHandler* menuInputHandler;
PostProcessor* effects;

// Temp
Helper::Stopwatch w1;
Helper::Stopwatch w2;
Helper::Stopwatch w3;

Forms::Form* form = nullptr;
Forms::Form* form_credit =  nullptr;

// Callbacks.
void onLayerDraw(const Tmx::Map *map, const Tmx::Layer *layer, int n_layer);
void onCameraScale(glm::vec2 scale);

Game::Game(unsigned int width, unsigned int height) 
	: State(GameState::active)
	, Width(width), Height(height)
	, Keys(), KeysProcessed()
	, BackgroundColor(0.0f)
	, Input(new InputInterface(Keys, KeysProcessed))
{}
Game::~Game()
{	
	if (renderer)
		delete renderer;
	if (tile_renderer)
		delete tile_renderer;
	if (basic_renderer)
		delete basic_renderer;
	if (text_renderer)
		delete text_renderer;
	if (menu)
		delete menu;
	if (menu_renderer)
		delete menu_renderer;
	if (menu_manager)
		delete menu_manager;
	if (levels_manager)
		delete levels_manager;
	if (menuInputHandler)
		delete menuInputHandler;
	if (atlas_text_renderer)
		delete atlas_text_renderer;
	if (form)
		delete form;
	if (form_credit)
		delete form_credit;
	if (effects)
		delete effects;
	
	mDialogs.clear();
	ResourceManager::Clear();

	delete Input;
}
void Game::SetTileSize(glm::vec2 new_size)
{
	Game::TileSize = new_size;
	TileSpace::CallOnTileSizeChanged(new_size);
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

	text_renderer->SetProjection(projection);

	Game::ProjectionMatrix = projection;
	Game::ScreenSize = glm::vec2(float(this->Width), float(this->Height));
	
	if (levels_manager->Active())
		levels_manager->ActiveLevel().OnResize();

	// Recenter forms.
	form->MoveTo(glm::vec2((Game::ScreenSize.x - form->vSize.x) * 0.5f, 80.0f));
	form_credit->MoveTo(glm::vec2(20.0f, Game::ScreenSize.y - 20.0f - form_credit->vSize.y));
	recenter_forms();

	effects->HandleResize(Width, Height);
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
	Game::SetTileSize(glm::vec2(32.0f));
	this->BackgroundColor = Helper::HexToRGB(0x2D3D1E);
	Game::ScreenSize = glm::vec2(float(Width), float(Height));
	// Projection used for 2D projection.
	glm::mat4 projection = glm::ortho(0.0f, (float)this->Width, (float)this->Height, 0.0f, -1.0f, 1.0f);
	Game::ProjectionMatrix = projection;

	// Load shaders
	ResourceManager::LoadShader(SHADERS_DIR "SpriteRender.vert", SHADERS_DIR "SpriteRender.frag", nullptr, "sprite");
	ResourceManager::LoadShader(SHADERS_DIR "tile_render.vert", SHADERS_DIR "tile_render.frag", nullptr, "tilemap");
	ResourceManager::LoadShader(SHADERS_DIR "BasicRender.vert", SHADERS_DIR "BasicRender.frag", nullptr, "basic_render");
	ResourceManager::LoadShader(SHADERS_DIR "post_processing.vert", SHADERS_DIR "post_processing.frag", nullptr, "effects");

	// Load textures
	ResourceManager::LoadTexture(ASSETS_DIR "textures/menu_9patch.png", true, "menu_9patch").SetMagFilter(GL_NEAREST).SetMinFilter(GL_NEAREST).UpdateParameters();
	Texture2D& font_atlas = ResourceManager::LoadTexture(ASSETS_DIR "fonts/atlas.png", true, "font_atlas").SetMagFilter(GL_NEAREST).SetMinFilter(GL_NEAREST).UpdateParameters();

	// Load levels manager
	levels_manager = new GameLevelsManager(ASSETS_DIR "Levels/levels.json");
	levels_manager->AddObserver(this);

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
	text_renderer->Load(ASSETS_DIR "fonts/arial.ttf", 32, GL_NEAREST);
	atlas_text_renderer = new AtlasTextRenderer();
	atlas_text_renderer->Load(font_atlas, glm::vec2(7.0f));

	// Initialize Camera. Note: Initialized further in GameLevel initialization.
	TileCamera2D::OnScale = onCameraScale;

	// Initialize tilemap renderer
	tile_renderer = new TilemapRenderer(ResourceManager::GetShader("tilemap"));
	tile_renderer->Projection = projection;

	/* ====== Initizlie menu objects ======= */
	// Initialize menus.
	menu = new MenuObject();
	menu->SetPatchSize(glm::ivec2(16));
	MenuObject& pauseMenu = menu->at("Pause Menu").SetTable(1, 4);
	pauseMenu["Resume"].SetID(RESUME_GAME_COMMAND);
	pauseMenu["Options"].SetTable(2, 2);
	pauseMenu["Options"]["option1"];
	pauseMenu["Options"]["option2"];
	pauseMenu["Options"]["option3"];
	pauseMenu["Options"]["option4"];
	pauseMenu["Options"]["option5"];
	pauseMenu["Options"]["option6"];
	pauseMenu["Options"]["option7"];
	pauseMenu["Restart"].SetID(RESTART_LEVEL_COMMAND);
	pauseMenu["Main Menu"].SetID(EXIT_TO_MAIN_MENU_COMMAND);

	MenuObject& mainMenu = menu->at("Main Menu").SetTable(1, 4);
	mainMenu["Play"].SetTable(1,  3);
	mainMenu["Options"];
	mainMenu["Reset progress"].SetID(RESET_PROGRESS_COMMAND);
	mainMenu["Exit game"].SetID(EXIT_GAME_COMMAND);
	auto& levelInfos = levels_manager->GetAllInfos();
	for (const auto& i : levelInfos) {
		auto progress = levels_manager->GetLevelProgress(i.nLevel);
		mainMenu["Play"][i.sName].Enable(!progress.bLocked).SetID(LOAD_LEVEL_COMMAND).SetCustomData(i.nLevel);
		if (progress.bCompleted)
			mainMenu["Play"][i.sName].SetTextColor(MENU_COMPLETED_LEVEL_COLOR);
	}

	menu->Build(atlas_text_renderer);

	// Initilize menu manager
	menu_manager = new MenuManager();

	// Initialize menu renderer
	menu_renderer = new MenuRenderer(renderer);

	// Initialize menu input handler
	menuInputHandler = new MenuInputHandler(menu_manager);

	// State options.
	State = GameState::main_menu;
	menu_manager->Open(&menu->at("Main Menu"));
	menu_manager->CloseOnBack(false);

	// ====== Initialize main menu forms ======
	form = new Forms::Form(atlas_text_renderer);
	form->nSpacing = 50;
	form->AddLabel("lblGameName", "Platformer Game!", glm::vec2(80.0f), Helper::HexToRGB(0x972E34));
	form->AddLabel("lblInfo", "beta", glm::vec2(32.0f), glm::vec3(0.1f, 0.9f, 0.8f));
	form->SetGravity(Forms::Gravity::center);
	form->MoveTo(glm::vec2((Game::ScreenSize.x - form->vSize.x) * 0.5f, 50.0f));

	form_credit = new Forms::Form(atlas_text_renderer);
	form_credit->AddPair("pairCredit",
		std::make_shared<Forms::Label>("By", glm::vec2(0.0f), glm::vec2(20.0f), glm::vec3(1.0f), atlas_text_renderer), 
		std::make_shared<Forms::Label>("Jakub Kloub", glm::vec2(0.0f), glm::vec2(25.0f), Helper::HexToRGB(0x5F4E7D), atlas_text_renderer));
	form_credit->MoveTo(glm::vec2(20.0f, Game::ScreenSize.y - 20.0f - form_credit->vSize.y));

	// Initialize game dialogs.
	init_dialogs();

	// Initialize effects.
	effects = new PostProcessor(ResourceManager::GetShader("effects"), Width, Height);
	effects->SetClearColor(BackgroundColor);
	effects->SetAnimStepMethod(AnimStepMethod::cubatic);
}

void Game::OnNotify(IObserverSubject* obj, int message, std::any args)
{
	Event e {obj, message, args};
	if (!eventQueue.empty() && eventQueue.front() == e)
		return;
	eventQueue.emplace(e);
}
void Game::ProcessInput(float dt)
{
	// Rotation functions used for testing of correct
	// positioning of objects.
	if (Input->Held(GLFW_KEY_Q))
		TileCamera2D::Rotate(glm::radians(45.0f) * dt);
	if (Input->Held(GLFW_KEY_E))
		TileCamera2D::Rotate(glm::radians(-45.0f) * dt);

	if (this->State == GameState::active)
	{
		levels_manager->ActiveLevel().ProcessInput(Input, dt);

		if (Input->Pressed(GLFW_KEY_SPACE))
		{
			TileCamera2D::SetRight(glm::vec2(1.0f, 0.0f));	
			TileCamera2D::SetScale(glm::vec2(2.0f));	
			Game::SetTileSize(Game::TileSize);	
		}
		if (Input->Pressed(GLFW_KEY_F2))
			render_aabb = !render_aabb;

		if (Input->Pressed(GLFW_KEY_ESCAPE))
		{
			this->State = GameState::ingame_paused;	
			menu_manager->Open(&menu->at("Pause Menu"));
			menu_manager->CloseOnBack(true);
		}
		if (Input->Pressed(GLFW_KEY_R))
			levels_manager->ActiveLevel().Restart();
	}
	else if (this->State == GameState::ingame_paused)
	{
		MenuObject* command = menuInputHandler->HandleInput(Input);
		if (command)
		{
			switch(command->GetID())
			{
			case RESUME_GAME_COMMAND:
				menu_manager->Close();
				this->State = GameState::active;
				break;
			case SAVE_GAME_COMMAND:
				levels_manager->Save();
				break;
			case EXIT_TO_MAIN_MENU_COMMAND:
				exit_to_main_menu();
				break;
			case EXIT_GAME_COMMAND:
				this->Run = false;
				break;
			case RESTART_LEVEL_COMMAND:
				levels_manager->ActiveLevel().Restart();
				State = GameState::active;
				menu_manager->Close();
				break;
			default:
				break;
			}
		}
		if (menu_manager->MenuClosed())
			this->State = GameState::active;
	}
	else if (this->State == GameState::main_menu)
	{
		MenuObject* command = menuInputHandler->HandleInput(Input);
		if (command)
		{
			switch (command->GetID())
			{
			case LOAD_LEVEL_COMMAND:
				effects->Restart([this, command](){
					levels_manager->Load(command->GetCustomData<int>());
					this->State = GameState::active;
					menu_manager->Close();
				});
				break;
			case EXIT_GAME_COMMAND:
				Run = false;
				break;
			case RESET_PROGRESS_COMMAND:
				levels_manager->DeleteSavedStatistics();
				break;
			default:
				break;
			}
		}
	}
	else if (this->State == GameState::ingame_dialog)
		pActiveDialog->ProcessInput(Input);


	if (Input->Pressed(GLFW_KEY_F1))
	{
		wireframe_render = !wireframe_render;
		if (wireframe_render)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if (Input->Pressed(GLFW_KEY_F3))
	{
		bRenderDebugInfo = !bRenderDebugInfo;
	}

}

void Game::handle_events()
{
	while (!eventQueue.empty())
	{
		Event e = eventQueue.front();
		eventQueue.pop();

		switch (e.message)
		{
		case PLAYER_DIED:
			State = GameState::ingame_dialog;
			pActiveDialog = &mDialogs["lost"];
			break;
		case PLAYER_REACHED_FINISH:
			State = GameState::ingame_dialog;
			pActiveDialog = &mDialogs["won"];
			update_win_dialog_info();
			break;
		case LEVEL_LOCKED_CHANGED: {
				MenuObject* level_entry = menu->at("Main Menu")["Play"].GetChildByCustomData<int>(e.args);
 				if (level_entry)
					level_entry->Enable(!level_entry->Enabled());
			}
			break;
		case LEVEL_COMPLETED_CHANGED: {
				MenuObject* level_entry = menu->at("Main Menu")["Play"].GetChildByCustomData<int>(e.args);
				if (level_entry)
					level_entry->SetTextColor(MENU_COMPLETED_LEVEL_COLOR);
			}
			break;
		case LEVEL_PROGRESS_RESETED: {
				auto& levelInfos = levels_manager->GetAllInfos();
				for (const auto& i : levelInfos) {
					auto progress = levels_manager->GetLevelProgress(i.nLevel);
					menu->at("Main Menu")["Play"][i.sName].Enable(!progress.bLocked);
					menu->at("Main Menu")["Play"][i.sName].SetTextColor(progress.bCompleted ? MENU_COMPLETED_LEVEL_COLOR : glm::vec3(1.0f));
				}
			}
			break;
		default:
			break;
		}
	}
}

void Game::Update(float dt)
{	
	w1.Restart();
	handle_events();
	if (State == GameState::active)
	{
		levels_manager->ActiveLevel().Update(dt);

		// Update Animations
		for (auto& [group, resources] : ResourceManager::AnimationManagers)
			for (auto& [name, resource] : resources)
				resource.obj->Update(dt);
	}
	effects->Update(dt);
	w1.Stop();

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
	if (!this->Run)
		return;

	effects->BeginRender();
	w2.Restart();
	if (State == GameState::active || State == GameState::ingame_paused || State == GameState::ingame_dialog)
	{
		levels_manager->ActiveLevel().Render(renderer, tile_renderer);
		// Render colliders.
		if (render_aabb)
		{
			auto world = levels_manager->ActiveLevel().PhysicsWorld;
			for (int i = 0; i < world->BodyCount(); i++)
			{
				auto body = world->GetBody(i);
				auto p = body->GetCollider();

				if (body->GetCollider()->GetType() == Physics2D::ColliderType::circle)
				{
					basic_renderer->RenderShape(br_Shape::circle_empty, 
						TileCamera2D::GetScreenPosition(p->GetAABB().GetMin(true)),
						p->GetAABB().GetSize(true) * Game::TileSize * TileCamera2D::GetScale(),
						0.0f, glm::vec3(1.0f, 1.0f, 1.0f)
					);
				}
				if (body->GetCollider()->GetType() != Physics2D::ColliderType::capsule)
				{
					basic_renderer->RenderClosedPolygon(
						p->UnitVertices,
						TileCamera2D::GetScreenPosition(p->GetCenter()),
						glm::vec2(world->GetInvMeterUnitRatio()) * Game::TileSize * TileCamera2D::GetScale(),
						glm::vec3(1.0f, 1.0f, 1.0f));
				}
				else 
				{
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
			// basic_renderer->RenderShape(br_Shape::rectangle, player->GetSprite()->Position, player->GetSprite()->Size, 0.0f, glm::vec3(1.0f, 1.0f, 0.0f));
		}
	}
	if (State == GameState::ingame_paused || State == GameState::main_menu)
	{
		glm::vec2 vMenuSize = menu_manager->First().GetTotalSize(2.0f);
		menu_renderer->Draw(*menu_manager, ResourceManager::GetTexture("menu_9patch"), (glm::vec2(Width, Height) - vMenuSize) * 0.5f, 2.0f);

		if (State == GameState::main_menu)
		{
			form->Render(renderer);	// Render game name.
			form_credit->Render(renderer);
		}
	}
	if (State == GameState::ingame_dialog)
	{
		pActiveDialog->Render(renderer);
	}
	w2.Stop();

	effects->EndRender();
	effects->Render();

	// Render DEBUG text
	if (bRenderDebugInfo)
	{
		char buf[256];
		sprintf(buf, "FPS: %.f\nUpdate step: %f ms\nRender step: %f ms",
			fps,
			w1.ElapsedMilliseconds(),
			w2.ElapsedMilliseconds()
		);
		text_renderer->RenderText(std::string(buf), Width - 200.0f, 10.0f, 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
	}
}

void Game::exit_to_main_menu()
{
	effects->Restart([&](){
		if (!levels_manager->Active())
			return;

		levels_manager->Save();
		levels_manager->Unload();
		State = GameState::main_menu;
		menu_manager->Close();
		menu_manager->Open(&menu->at("Main Menu"));
		menu_manager->CloseOnBack(false);
		this->eventQueue.emplace(Event{ nullptr, MAIN_MENU_LOADED, nullptr});
	});
}


void Game::init_dialogs()
{
	// ==== You Won Form definition ====
	auto form = std::make_shared<Forms::Form>(atlas_text_renderer);
	form->AddLabel("lblWon", "You won!", glm::vec2(64.0f), Helper::HexToRGB(0xE0BA1E));
	form->AddPair("pairCoins", 
        std::make_shared<Forms::Label>("Coins", glm::vec2(0.0f), glm::vec2(28.0f), glm::vec3(1.0f, 1.0f, 0.0f), atlas_text_renderer), 
        std::make_shared<Forms::Label>("0/0", glm::vec2(0.0f), glm::vec2(36.0f), glm::vec3(1.0f, 1.0f, 1.0f), atlas_text_renderer)
    );

	form->AddPair("pairTime", 
        std::make_shared<Forms::Label>(" Time", glm::vec2(0.0f), glm::vec2(28.0f), glm::vec3(0.0f, 0.7f, 0.08f), atlas_text_renderer), 
        std::make_shared<Forms::Label>("0s", glm::vec2(0.0f), glm::vec2(36.0f), glm::vec3(1.0f, 1.0f, 1.0f), atlas_text_renderer)
    );

	auto row = form->AddControl("rwInputRestart", std::make_shared<Forms::Row>(glm::vec2(0.0f, 0.0f))).get<Forms::Row*>();
	row->AddControls({
        { "lblTextP1", std::make_shared<Forms::Label>("Press",      glm::vec2(0.0f), glm::vec2(28.0f), glm::vec3(1.0f), atlas_text_renderer), Forms::ControlType::label },
        { "lblTextP2", std::make_shared<Forms::Label>("r",          glm::vec2(0.0f), glm::vec2(36.0f), glm::vec3(0.0f, 1.0f, 0.0f), atlas_text_renderer), Forms::ControlType::label },
        { "lblTextP3", std::make_shared<Forms::Label>("to restart", glm::vec2(0.0f), glm::vec2(28.0f), glm::vec3(1.0f), atlas_text_renderer), Forms::ControlType::label }
    });
	row->SetGravity(Forms::Gravity::center);

	row = form->AddControl("rwInputMainMenu", std::make_shared<Forms::Row>(glm::vec2(0.0f))).get<Forms::Row*>();
	row->AddControls({
		{ "lblTextP1", std::make_shared<Forms::Label>("Press", 		   glm::vec2(0.0f), glm::vec2(28.0f), glm::vec3(1.0f), atlas_text_renderer), Forms::ControlType::label },
		{ "lblTextP2", std::make_shared<Forms::Label>("M", 			   glm::vec2(0.0f), glm::vec2(36.0f), glm::vec3(0.0f, 1.0f, 0.0f), atlas_text_renderer), Forms::ControlType::label },
		{ "lblTextP3", std::make_shared<Forms::Label>("for main menu", glm::vec2(0.0f), glm::vec2(28.0f), glm::vec3(1.0f), atlas_text_renderer), Forms::ControlType::label }
	});
    row->SetGravity(Forms::Gravity::center);

	row = form->AddControl("rwInputNextLevel", std::make_shared<Forms::Row>(glm::vec2(0.0f))).get<Forms::Row*>();
	row->AddControls({
		{ "lblTextP1", std::make_shared<Forms::Label>("Press", 		   glm::vec2(0.0f), glm::vec2(28.0f), glm::vec3(1.0f), atlas_text_renderer), Forms::ControlType::label },
		{ "lblTextP2", std::make_shared<Forms::Label>("N", 			   glm::vec2(0.0f), glm::vec2(36.0f), glm::vec3(0.0f, 1.0f, 0.0f), atlas_text_renderer), Forms::ControlType::label },
		{ "lblTextP3", std::make_shared<Forms::Label>("for next level", glm::vec2(0.0f), glm::vec2(28.0f), glm::vec3(1.0f), atlas_text_renderer), Forms::ControlType::label }
	});
    row->SetGravity(Forms::Gravity::center);

	mDialogs["won"] = Dialog(form, {
		{GLFW_KEY_R, [&](){ levels_manager->ActiveLevel().Restart(); State = GameState::active; }},
		{GLFW_KEY_M, std::bind(&Game::exit_to_main_menu, this)},
		{GLFW_KEY_N, [&](){ 
			int active = levels_manager->ActiveLevelIndex();
			if (int(levels_manager->GetAllInfos().size()) == active + 1)
				return;
			levels_manager->Save();
			levels_manager->Load(active + 1);
			State = GameState::active; 
		}}
	});

	form->SetGravity(Forms::Gravity::left);

	// ==== You lost form definition ====
	form = std::make_shared<Forms::Form>(atlas_text_renderer);
	form->AddLabel("lblLost", "You lost!", glm::vec2(80.0f), Helper::HexToRGB(0xE0BA1E));
	form->AddLabel("lblLostText1", "oh thats sad", glm::vec2(20.0f), glm::vec3(1.0f));
	form->AddLabel("lblLostText2", "you should try again", glm::vec2(20.0f), glm::vec3(1.0f));

	row = form->AddControl("rwInputRestart", std::make_shared<Forms::Row>(glm::vec2(0.0f, 0.0f))).get<Forms::Row*>();
	row->AddControls({
        { "lblTextP1", std::make_shared<Forms::Label>("Restart", 	glm::vec2(0.0f), glm::vec2(28.0f), glm::vec3(1.0f), atlas_text_renderer), Forms::ControlType::label },
		{ "lblTextP2", std::make_shared<Forms::Label>("r ", 		glm::vec2(0.0f), glm::vec2(36.0f), glm::vec3(0.0f, 1.0f, 0.0f), atlas_text_renderer), Forms::ControlType::label },
        { "lblTextP3", std::make_shared<Forms::Label>("Main menu",	glm::vec2(0.0f), glm::vec2(28.0f), glm::vec3(1.0f), atlas_text_renderer), Forms::ControlType::label },
		{ "lblTextP4", std::make_shared<Forms::Label>("m", 			glm::vec2(0.0f), glm::vec2(36.0f), glm::vec3(0.0f, 1.0f, 0.0f), atlas_text_renderer), Forms::ControlType::label }
    });
	row->SetGravity(Forms::Gravity::center);
	form->SetGravity(Forms::Gravity::center);

	mDialogs["lost"] = Dialog(form, {
		{GLFW_KEY_R, [&](){ levels_manager->ActiveLevel().Restart(); State = GameState::active; }},
		{GLFW_KEY_M, std::bind(&Game::exit_to_main_menu, this)}
	});	
}
void Game::recenter_forms()
{
	for (auto& dialog : mDialogs)
	{
		std::shared_ptr<Forms::Form>& form = dialog.second.GetForm();
		form->MoveTo((Game::ScreenSize - form->vSize) * 0.5f);
	}
}

void Game::update_win_dialog_info()
{
	GameLevel& pActiveLevel = levels_manager->ActiveLevel();
	std::string sCoins = std::to_string(pActiveLevel.GetCoins()) + "/" + std::to_string(pActiveLevel.GetCoinsTotal());
	std::string sTime = std::to_string(int(pActiveLevel.GetElapsedTime())) + "s";

	mDialogs["won"]->GetControl<Forms::Pair*>("pairCoins")->GetSecond<Forms::Label*>()->SetText(sCoins);
	mDialogs["won"]->GetControl<Forms::Pair*>( "pairTime")->GetSecond<Forms::Label*>()->SetText(sTime);
}

// Callbacks
void onCameraScale(glm::vec2 scale)
{
	tile_renderer->HandleScale(scale);
}
